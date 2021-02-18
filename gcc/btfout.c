/* Output BTF format from GCC.
   Copyright (C) 2021 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 3, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

/* This contains routines to output the BPF Type Format (BTF). The BTF debug
   format is very similar to CTF; as a result, the structure of this file
   closely resembles that of ctfout.c, and the same CTF container objects
   are used.  */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "target.h"
#include "output.h"
#include "dwarf2asm.h"
#include "debug.h"
#include "ctfc.h"
#include "diagnostic-core.h"

static int btf_label_num;

static GTY (()) section * btf_info_section;

/* BTF debug info section.  */

#ifndef BTF_INFO_SECTION_NAME
#define BTF_INFO_SECTION_NAME  ".BTF"
#endif

#define BTF_INFO_SECTION_FLAGS (SECTION_DEBUG)

/* Maximum size (in bytes) for an artifically generated BTF label.  */

#define MAX_BTF_LABEL_BYTES 40

static char btf_info_section_label[MAX_BTF_LABEL_BYTES];

#ifndef BTF_INFO_SECTION_LABEL
#define BTF_INFO_SECTION_LABEL  "Lbtf"
#endif

/* BTF encodes void as type id 0.  */

#define BTF_VOID_TYPEID 0
#define BTF_INIT_TYPEID 1

/* Map a CTF type kind to the corresponding BTF type kind.  */

static unsigned int
get_btf_kind (unsigned int ctf_kind)
{
  /* N.B. the values encoding kinds are not in general the same for the
     same kind between CTF and BTF. e.g. CTF_K_CONST != BTF_KIND_CONST.  */
  switch (ctf_kind)
    {
    case CTF_K_INTEGER:  return BTF_KIND_INT;
    case CTF_K_POINTER:  return BTF_KIND_PTR;
    case CTF_K_ARRAY:    return BTF_KIND_ARRAY;
    case CTF_K_FUNCTION: return BTF_KIND_FUNC_PROTO;
    case CTF_K_STRUCT:   return BTF_KIND_STRUCT;
    case CTF_K_UNION:    return BTF_KIND_UNION;
    case CTF_K_ENUM:     return BTF_KIND_ENUM;
    case CTF_K_FORWARD:  return BTF_KIND_FWD;
    case CTF_K_TYPEDEF:  return BTF_KIND_TYPEDEF;
    case CTF_K_VOLATILE: return BTF_KIND_VOLATILE;
    case CTF_K_CONST:    return BTF_KIND_CONST;
    case CTF_K_RESTRICT: return BTF_KIND_RESTRICT;
    default:;
    }
  return BTF_KIND_UNKN;
}

/* Each BTF type can be followed additional, variable-length information
   completing the description of the type. Calculate the number of bytes
   of variable information required to encode a given type.  */

static unsigned long
btf_calc_num_vbytes (ctf_dtdef_ref ctftype)
{
  unsigned long vlen_bytes = 0;

  uint32_t ctfkind = CTF_V2_INFO_KIND (ctftype->dtd_data.ctti_info);
  uint32_t vlen = CTF_V2_INFO_VLEN (ctftype->dtd_data.ctti_info);

  uint32_t kind = get_btf_kind (ctfkind);

  ctf_dmdef_t * dmd;
  unsigned int num_members = 0;

  switch (kind)
    {
    case BTF_KIND_UNKN:
    case BTF_KIND_PTR:
    case BTF_KIND_FWD:
    case BTF_KIND_TYPEDEF:
    case BTF_KIND_VOLATILE:
    case BTF_KIND_CONST:
    case BTF_KIND_RESTRICT:
    case BTF_KIND_FUNC:
    /* These kinds have no vlen data.  */
      break;

    case BTF_KIND_INT:
      /* Size 0 integers represent redundant definitions of void that will
	 not be emitted. Don't allocate space for them.  */
      if (ctftype->dtd_data.ctti_size == 0)
	break;

      vlen_bytes += sizeof (uint32_t);
      break;

    case BTF_KIND_ARRAY:
      vlen_bytes += sizeof (struct btf_array);
      break;

    case BTF_KIND_STRUCT:
    case BTF_KIND_UNION:
      /* Sanity check vlen == number of struct/union members.  */
      for (dmd = ctftype->dtd_u.dtu_members;
	   dmd != NULL; dmd = (ctf_dmdef_t *) ctf_dmd_list_next (dmd))
	num_members++;
      gcc_assert (vlen == num_members);

      vlen_bytes += num_members * sizeof (struct btf_member);
      break;

    case BTF_KIND_ENUM:
      vlen_bytes += vlen * sizeof (struct btf_enum);
      break;

    case BTF_KIND_FUNC_PROTO:
      vlen_bytes += vlen * sizeof (struct btf_param);
      break;

    case BTF_KIND_VAR:
      vlen_bytes += sizeof (struct btf_var);
      break;

    case BTF_KIND_DATASEC:
      vlen_bytes += vlen * sizeof (struct btf_var_secinfo);
      break;

    default:
      break;
    }
  return vlen_bytes;
}


void
init_btf_sections (void)
{
  btf_info_section = get_section (BTF_INFO_SECTION_NAME, BTF_INFO_SECTION_FLAGS,
				  NULL);

  ASM_GENERATE_INTERNAL_LABEL (btf_info_section_label,
			       BTF_INFO_SECTION_LABEL, btf_label_num++);
}

/* Holes occur for types which are present in the CTF container, but are either
   non-representable or redundant in BTF.  */

static vec<ctf_id_t> holes;

/* CTF definition(s) of void. Only one definition of void should be generated.  */

static vec<ctf_id_t> voids;

static vec<ctf_dtdef_ref> funcs;

/* Adjust the given ID to account for holes and duplicate definitions of void.  */

static ctf_id_t
btf_adjust_type_id (ctf_id_t id)
{
  size_t n;
  for (n = 0; n < voids.length (); n++)
    if (id == voids[n])
      return BTF_VOID_TYPEID;

  for (n = 0; n < holes.length (); n++)
    if (holes[n] >= id)
      break;

  return id - n;
}

/* Adjust any references this type makes to other types, to account for
   removed types.  */

static void
btf_adjust_type_ref (ctf_dtdef_ref dtd)
{
  unsigned int ctf_kind = CTF_V2_INFO_KIND (dtd->dtd_data.ctti_info);
  unsigned int btf_kind = get_btf_kind (ctf_kind);

  switch (btf_kind)
    {
    case BTF_KIND_FUNC_PROTO:
      {
	/* Adjust each argument type.  */
	ctf_func_arg_t * farg;
	for (farg = dtd->dtd_u.dtu_argv;
	     farg != NULL; farg = (ctf_func_arg_t *) ctf_farg_list_next (farg))
	  farg->farg_type = btf_adjust_type_id (farg->farg_type);

	/* Adjust return type.  */
	dtd->dtd_data.ctti_type = btf_adjust_type_id (dtd->dtd_data.ctti_type);
	break;
      }
    case BTF_KIND_STRUCT:
    case BTF_KIND_UNION:
      {
	/* Structs/unions hold a type reference for each member.  */
	ctf_dmdef_t * dmd;
	for (dmd = dtd->dtd_u.dtu_members;
	     dmd != NULL; dmd = (ctf_dmdef_t *) ctf_dmd_list_next (dmd))
	  dmd->dmd_type = btf_adjust_type_id (dmd->dmd_type);
	break;
      }
    case BTF_KIND_ARRAY:
      {
	/* Arrays hold two type references: the element type and index type.  */
	ctf_arinfo_t *arinfo = &(dtd->dtd_u.dtu_arr);
	arinfo->ctr_contents = btf_adjust_type_id (arinfo->ctr_contents);
	arinfo->ctr_index = btf_adjust_type_id (arinfo->ctr_contents);
	break;
      }
    case BTF_KIND_PTR:
    case BTF_KIND_TYPEDEF:
    case BTF_KIND_VOLATILE:
    case BTF_KIND_CONST:
    case BTF_KIND_RESTRICT:
    case BTF_KIND_FUNC:
    case BTF_KIND_VAR:
      /* These types hold just the reference in the base dtdef.  */
      dtd->dtd_data.ctti_type = btf_adjust_type_id (dtd->dtd_data.ctti_type);
      break;

    default:
      /* Other types (like integers) don't reference any types.  */
      break;
    }
}

static void
btf_adjust_var_ref (ctf_dvdef_ref dvd)
{
  dvd->dvd_type = btf_adjust_type_id (dvd->dvd_type);
}

static void
btf_adjust_refs (ctf_container_ref ctfc)
{

/* The second pass over the BTF types list. Adjust type IDs to account for the
   holes. Types which become holes are not actually removed from the list yet.
   Types which reference a type after the Nth hole have their reference adjusted
   by -N.
   Note that there is no need to adjust the IDs of the types themselves; the
   IDs stored in the type are not used once the list is constructed.  */

  size_t i;
  size_t num_types = ctfc->ctfc_types->elements ();
  size_t num_vars = ctfc->ctfc_vars->elements ();
  if (num_types)
    {
      for (i = 1; i <= num_types; i++)
	{
	  ctf_dtdef_ref dtd = ctfc->ctfc_types_list[i];
	  btf_adjust_type_ref (dtd);
	}
    }

  if (num_vars)
    {
      for (i = 0; i < num_vars; i++)
	{
	  ctf_dvdef_ref dvd = ctfc->ctfc_vars_list[i];
	  btf_adjust_var_ref (dvd);
	}
    }
  for (i = 0; i < funcs.length (); i++)
    btf_adjust_type_ref (funcs[i]);
}

static void
btf_preprocess_type (ctf_container_ref ctfc, ctf_dtdef_ref type)
{
  ctf_dtdef_ref ref_type;
  unsigned int ctf_kind, btf_kind;

  ctf_kind = CTF_V2_INFO_KIND (type->dtd_data.ctti_info);
  btf_kind = get_btf_kind (ctf_kind);

  if (btf_kind == BTF_KIND_UNKN)
    /* This type is not representable in BTF. Create a hole.  */
    holes.safe_push (type->dtd_type);

  else if (btf_kind == BTF_KIND_INT && type->dtd_data.ctti_size == 0)
    {
      /* This is a (redundant) definition of void.  */
      voids.safe_push (type->dtd_type);
      holes.safe_push (type->dtd_type);
    }

  else if (btf_kind == BTF_KIND_FUNC_PROTO)
    {
      /* Functions actually get two types: a BTF_KIND_FUNC_PROTO, and
	 also a BTF_KIND_FUNC. But the CTF container only allocates one
	 type per function, which matches closely with BTF_KIND_FUNC_PROTO.
	 For each such function, also allocate a BTF_KIND_FUNC entry.
	 These will be output later.  */
      ctf_dtdef_ref func_dtd = ggc_cleared_alloc<ctf_dtdef_t> ();
      func_dtd->dtd_data = type->dtd_data;
      func_dtd->dtd_data.ctti_type = type->dtd_type;

      funcs.safe_push (func_dtd);

      /* Only the BTF_KIND_FUNC type actually references the name. The
	 BTF_KIND_FUNC_PROTO is always anonymous.  */
      type->dtd_data.ctti_name = 0;
    }

  /* Bitfields.  */
  else if (btf_kind == BTF_KIND_STRUCT || btf_kind == BTF_KIND_UNION)
    {
      ctf_dmdef_t *dmd;

      for (dmd = type->dtd_u.dtu_members;
	   dmd != NULL; dmd = (ctf_dmdef_t *) ctf_dmd_list_next (dmd))
	{
	  /* Use the dmd_value field, which is otherwise only used for enum
	     values, to keep track of bitfield members.  */
	  dmd->dmd_value = 0;

	  ref_type = ctfc->ctfc_types_list [dmd->dmd_type];
	  if (CTF_V2_INFO_KIND (ref_type->dtd_data.ctti_info) == CTF_K_SLICE)
	    {
	      ctf_id_t base = ref_type->dtd_u.dtu_slice.cts_type;

	      unsigned short word_offset = ref_type->dtd_u.dtu_slice.cts_offset;
	      unsigned short bits = ref_type->dtd_u.dtu_slice.cts_bits;
	      unsigned long sou_offset = dmd->dmd_offset;

	      /* FIXME - these should emit an error instead of assert.
		 For struct bitfield members, we only have 24 bit offset.  */
	      gcc_assert (sou_offset + word_offset <= 0xffffff);
	      gcc_assert (bits <= 0xff);

	      /* Pack the bit offset and bitfield size together.  */
	      dmd->dmd_offset += word_offset;
	      dmd->dmd_offset &= 0x00ffffff;
	      dmd->dmd_offset |= ((bits & 0xff) << 24);

	      /* Mark the member as a bitfield for later convenience.  */
	      dmd->dmd_value = 1;

	      /* Point to the base type of the slice rather than the slice.  */
	      dmd->dmd_type = base;
	    }
	}
    }
}

bool
btf_dvd_preprocess_cb (dw_die_ref const & ARG_UNUSED (key), ctf_dvdef_ref *slot,
		       ctf_container_ref arg_ctfc)
{
  ctf_dvdef_ref var = (ctf_dvdef_ref) * slot;

  /* FIXME - static may not fly with multiple CUs.  */
  static int num_vars_added = 0;
  arg_ctfc->ctfc_vars_list[num_vars_added++] = var;

  return 1;
}

bool
btf_dtd_preprocess_cb (dw_die_ref const & ARG_UNUSED (key), ctf_dtdef_ref *slot,
		       ctf_container_ref arg_ctfc)
{
  ctf_dtdef_ref ctftype = (ctf_dtdef_ref) * slot;

  size_t index = ctftype->dtd_type;
  gcc_assert (index <= arg_ctfc->ctfc_types->elements ());

  arg_ctfc->ctfc_types_list[index] = ctftype;

  arg_ctfc->ctfc_num_vlen_bytes += btf_calc_num_vbytes (ctftype);

  return 1;
}

/* Preprocess the CTF information to prepare for BTF output.
   BTF is almost a subset of CTF, with many small differences in encoding, and
   lacking support for some types (notably floating point formats).

   The preprocess pass:
   - Types are sorted in order of their type IDs. The BTF types section is
     viewed as an array, with type IDs used to index into that array. It is
     essential that every type be placed at the exact index corresponding to
     its ID, or else references to that type from other types will no longer
     be correct.

   - Convert bitfield struct/union members to BTF encoding. CTF uses slices to
     encode bitfields, but BTF does not have slices and encodes bitfield
     information directly in the variable-length btf_member descriptions
     following the struct or union type.

   - Remove non-representable types. We cannot have any invalid BTF types
     appearing in the output so they must be removed, and type ids of other
     types and references adjust accordingly.

   - Convert references to a void type to type ID 0. CTF uses a unique type to
     encode void, but BTF simply uses type ID 0.  */

static void
btf_preprocess (ctf_container_ref ctfc)
{
  size_t num_ctf_types = ctfc->ctfc_types->elements ();
  size_t num_ctf_vars = ctfc->ctfc_vars->elements ();
  unsigned int i;

  if (num_ctf_types)
    {
      /* Allocate the types list and traverse all types, placing each type
	 at the index according to its ID. Add 1 because type ID 0 always
	 represents VOID.  */
      ctfc->ctfc_types_list = ggc_vec_alloc<ctf_dtdef_ref>(num_ctf_types + 1);
      ctfc->ctfc_types->traverse<ctf_container_ref, btf_dtd_preprocess_cb>
	(ctfc);

      /* After the initial list of types has been built, we can perform
	 preprocessing which traverses links between types.  */
      for (i = 1; i <= num_ctf_types; i++)
	{
	  ctf_dtdef_ref type = ctfc->ctfc_types_list [i];
	  btf_preprocess_type (ctfc, type);
	}
    }

  if (num_ctf_vars)
    {
      /* Allocate and construct the list of variables. While BTF variables are
	 not distinct from types (in that variables are simply types with
	 BTF_KIND_VAR), it is simpler to maintain a separate list of variables
	 and append them to the types list during output.  */
      ctfc->ctfc_vars_list = ggc_vec_alloc<ctf_dvdef_ref>(num_ctf_vars);
      ctfc->ctfc_vars->traverse<ctf_container_ref, btf_dvd_preprocess_cb> (ctfc);

      ctfc->ctfc_num_vlen_bytes += (num_ctf_vars * sizeof (struct btf_var));
    }

  btf_adjust_refs (ctfc);

  /* Account for number of holes so that the final type count is accurate.  */
  if (ctfc->ctfc_num_stypes >= holes.length ())
    ctfc->ctfc_num_stypes -= holes.length ();
  else
    ctfc->ctfc_num_stypes = 0;
}

/* BTF asm helper routines.  */

/* Asm'out a BTF type. This routine is responsible for the bulk of the task
   of converting CTF types to their BTF representation.  */

static void
btf_asm_type (ctf_dtdef_ref type)
{
  unsigned int btf_kind, btf_kflag, btf_vlen, btf_size_type;
  unsigned int ctf_info = type->dtd_data.ctti_info;
  unsigned int btf_info;

  btf_kind = get_btf_kind (CTF_V2_INFO_KIND (ctf_info));
  btf_size_type = type->dtd_data.ctti_type;

  /* By now any unrepresentable types have been removed or handled. Skip.  */
  if (btf_kind == BTF_KIND_UNKN)
    return;

  /* Size 0 integers are redundant definitions of void. References to them
     have been adjusted to point to BTF_VOID_TYPEID. Skip.  */
  if (btf_kind == BTF_KIND_INT && btf_size_type < 1)
    return;

  /* Re-encode the ctti_info to BTF.  */
  /* kflag is 1 for structs/unions with a bitfield member.
     kflag is 1 for forwards to unions.
     kflag is 0 in all other cases. */
  btf_kflag = 0;

  if (btf_kind == BTF_KIND_STRUCT || btf_kind == BTF_KIND_UNION)
    {
      /* If a struct/union has ANY bitfield members, set kflag=1.
	 Note that we must also change the encoding of every member to encode
	 both member bitfield size (stealing most-significant 8 bits) and bit
	 offset (LS 24 bits). This is done during preprocessing.  */
      ctf_dmdef_t *dmd;
      for (dmd = type->dtd_u.dtu_members;
	   dmd != NULL; dmd = (ctf_dmdef_t *) ctf_dmd_list_next (dmd))
	{
	  /* During preprocessing, we mark bitfield members by using the
	     dmd_value field, which is normally only used for enumerators.  */
	  if (dmd->dmd_value)
	    {
	      btf_kflag = 1;
	      break;
	    }
	}
    }

  /* BTF forwards make use of KIND_FLAG to distinguish between forwards to
     structs and forwards to unions. The dwarf2ctf conversion process stores
     the kind of the forward in ctti_type, but for BTF this must be 0 for
     forwards, with only the KIND_FLAG to distinguish.
     At time of writing, BTF forwards to enums are unspecified.  */
  if (btf_kind == BTF_KIND_FWD)
    {
      if (type->dtd_data.ctti_type == CTF_K_UNION)
	btf_kflag = 1;

      btf_size_type = 0;
    }

  /* Encode the type information.  */
  btf_vlen = CTF_V2_INFO_VLEN (ctf_info);
  btf_info = BTF_TYPE_INFO (btf_kind, btf_kflag, btf_vlen);

  dw2_asm_output_data (4, type->dtd_data.ctti_name, "btt_name");
  dw2_asm_output_data (4, btf_info, "btt_info");
  dw2_asm_output_data (4, btf_size_type, "btt_size or btt_type");
}

static void
btf_asm_array (ctf_dtdef_ref dtd)
{
  dw2_asm_output_data (4, dtd->dtd_u.dtu_arr.ctr_contents, "bta_contents");
  dw2_asm_output_data (4, dtd->dtd_u.dtu_arr.ctr_index, "bta_index");
  dw2_asm_output_data (4, dtd->dtd_u.dtu_arr.ctr_nelems, "bta_nelems");
}

static void
btf_asm_varent (ctf_dvdef_ref var)
{
  dw2_asm_output_data (4, var->dvd_name_offset, "btv_name");
  dw2_asm_output_data (4, BTF_TYPE_INFO (BTF_KIND_VAR, 0, 0), "btv_info");
  dw2_asm_output_data (4, var->dvd_type, "btv_type");
  /* FIXME - struct btf_var - 0=static or 1=global.  */
  dw2_asm_output_data (4, 1, "btv_linkage");
}

static void
btf_asm_sou_member (ctf_dmdef_t * dmd)
{
  /* N.B. just like a ctf member but with type/offset order swapped.  */
  dw2_asm_output_data (4, dmd->dmd_name_offset, "btm_name_off");
  dw2_asm_output_data (4, dmd->dmd_type, "btm_type");
  dw2_asm_output_data (4, dmd->dmd_offset, "btm_offset");
}

static void
btf_asm_enum_const (ctf_dmdef_t * dmd)
{
  dw2_asm_output_data (4, dmd->dmd_name_offset, "bte_name");
  dw2_asm_output_data (4, dmd->dmd_value, "bte_value");
}

static void
btf_asm_func_arg (ctf_func_arg_t * farg, size_t stroffset)
{
  /* If the function arg does not have a name, refer to the null string at
     the start of the string table. This ensures correct encoding for varargs
     '...' arguments.  */
  if ((farg->farg_name != NULL) && strcmp (farg->farg_name, ""))
    dw2_asm_output_data (4, farg->farg_name_offset + stroffset, "farg_name");
  else
    dw2_asm_output_data (4, 0, "farg_name");

  dw2_asm_output_data (4, farg->farg_type, "farg_type");
}

static void
btf_asm_func_type (ctf_dtdef_ref dtd)
{
  dw2_asm_output_data (4, dtd->dtd_data.ctti_name, "btt_name");
  dw2_asm_output_data (4, BTF_TYPE_INFO (BTF_KIND_FUNC, 0, 0), "btt_info");
  dw2_asm_output_data (4, dtd->dtd_data.ctti_type, "btt_size or btt_type");
}

static void
output_btf_header (ctf_container_ref ctfc)
{
   switch_to_section (btf_info_section);
   ASM_OUTPUT_LABEL (asm_out_file, btf_info_section_label);

   /* BTF magic number, version, flags, and header length.  */
   dw2_asm_output_data (2, BTF_MAGIC, "btf_magic");
   dw2_asm_output_data (1, BTF_VERSION, "btf_version");
   dw2_asm_output_data (1, 0, "btf_flags");
   /* struct btf_header defined in <btf.h> */
   dw2_asm_output_data (4, sizeof (struct btf_header), "btf_hdr_len");

   uint32_t type_off = 0, type_len = 0, num_vars;
   uint32_t str_off = 0, str_len = 0;
   if (! is_empty_container (ctfc))
     {
       num_vars = get_num_ctf_vars (ctfc);

       /* ctf types, stypes, and vars all map to btf types */
       type_len = (ctfc->ctfc_num_stypes * sizeof (struct btf_type))
	 + (num_vars * sizeof (struct btf_type))
	 + (funcs.length() * sizeof (struct btf_type))
	 + ctfc->ctfc_num_vlen_bytes;

       str_off = type_off + type_len;

       str_len = ctfc->ctfc_strlen + ctfc->ctfc_aux_strlen;
     }

   /* Offset of type section.  */
   dw2_asm_output_data (4, type_off, "type_off");
   /* Length of type section in bytes.  */
   dw2_asm_output_data (4, type_len, "type_len");
    /* Offset of string section.  */
   dw2_asm_output_data (4, str_off, "str_off");
    /* Length of string section in bytes.  */
   dw2_asm_output_data (4, str_len, "str_len");
}

static void
output_btf_vars (ctf_container_ref ctfc)
{
  size_t i;
  size_t num_ctf_vars = ctfc->ctfc_vars->elements ();
  if (num_ctf_vars)
    {
      for (i = 0; i < num_ctf_vars; i++)
	btf_asm_varent (ctfc->ctfc_vars_list[i]);
    }
}

/* Output BTF string records. The BTF strings section is a concatenation
   of the standard and auxilliary string tables in the ctf container. */

static void
output_btf_strs (ctf_container_ref ctfc)
{
  ctf_string_t * ctf_string = ctfc->ctfc_strtable.ctstab_head;

  while (ctf_string)
    {
      dw2_asm_output_nstring (ctf_string->cts_str, -1, "btf_string");
      ctf_string = ctf_string->cts_next;
    }

  ctf_string = ctfc->ctfc_aux_strtable.ctstab_head;
  while (ctf_string)
    {
      dw2_asm_output_nstring (ctf_string->cts_str, -1, "btf_aux_string");
      ctf_string = ctf_string->cts_next;
    }
}

static void
output_asm_btf_sou_fields (ctf_container_ref ARG_UNUSED (ctfc),
			   ctf_dtdef_ref dtd)
{
  ctf_dmdef_t * dmd;

  for (dmd = dtd->dtd_u.dtu_members;
       dmd != NULL; dmd = (ctf_dmdef_t *) ctf_dmd_list_next (dmd))
    btf_asm_sou_member (dmd);

}


static void
output_asm_btf_enum_list (ctf_container_ref ARG_UNUSED (ctfc),
			  ctf_dtdef_ref dtd)
{
  ctf_dmdef_t * dmd;

  for (dmd = dtd->dtd_u.dtu_members;
       dmd != NULL; dmd = (ctf_dmdef_t *) ctf_dmd_list_next (dmd))
    btf_asm_enum_const (dmd);
}

static void
output_asm_btf_func_args_list (ctf_container_ref ctfc,
			       ctf_dtdef_ref dtd)
{
  size_t farg_name_offset = ctfc->ctfc_strlen;
  ctf_func_arg_t * farg;
  for (farg = dtd->dtd_u.dtu_argv;
       farg != NULL; farg = (ctf_func_arg_t *) ctf_farg_list_next (farg))
    btf_asm_func_arg (farg, farg_name_offset);
}

static void
output_asm_btf_vlen_bytes (ctf_container_ref ctfc, ctf_dtdef_ref ctftype)
{

  uint32_t ctfkind = CTF_V2_INFO_KIND (ctftype->dtd_data.ctti_info);
  uint32_t btf_kind, encoding;

  btf_kind = get_btf_kind (ctfkind);

  if (btf_kind == BTF_KIND_UNKN)
    return;

  switch (btf_kind)
    {
    case BTF_KIND_INT:
      /* Redundant definitions of void may still be hanging around in the type
	 list as size 0 integers. Skip emitting them.  */
      if (ctftype->dtd_data.ctti_size < 1)
	break;

      encoding = BTF_INT_DATA (ctftype->dtd_u.dtu_enc.cte_format,
			       ctftype->dtd_u.dtu_enc.cte_offset,
			       ctftype->dtd_u.dtu_enc.cte_bits);

      dw2_asm_output_data (4, encoding, "bti_encoding");
      break;

    case BTF_KIND_ARRAY:
      btf_asm_array (ctftype);
      break;

    case BTF_KIND_STRUCT:
    case BTF_KIND_UNION:
      output_asm_btf_sou_fields (ctfc, ctftype);
      break;

    case BTF_KIND_ENUM:
      output_asm_btf_enum_list (ctfc, ctftype);
      break;

    case BTF_KIND_FUNC_PROTO:
      output_asm_btf_func_args_list (ctfc, ctftype);
      break;

    case BTF_KIND_VAR:
      /* BTF Variables are handled by output_btf_vars and btf_asm_varent.
	 There should be no BTF_KIND_VAR types at this point.  */
      gcc_assert (0);
      break;

    case BTF_KIND_DATASEC:
      /* FIXME BTF DATASEC is followed by info.vlen number of
	 struct btf_var_sectinfo {
	 u32 type;
	 u32 offset;
	 u32 size
	 }  */
      break;

    default:
      /* All other BTF type kinds have no variable length data.  */
      break;
    }
}

static void
output_asm_btf_type (ctf_container_ref ctfc, ctf_dtdef_ref type)
{
  btf_asm_type (type);
  output_asm_btf_vlen_bytes (ctfc, type);
}

static void
output_btf_types (ctf_container_ref ctfc)
{
  size_t i;
  size_t num_types = ctfc->ctfc_types->elements ();
  if (num_types)
    {
      for (i = 1; i <= num_types; i++)
	output_asm_btf_type (ctfc, ctfc->ctfc_types_list[i]);
    }
}

static void
output_btf_func_types (void)
{
  for (size_t i = 0; i < funcs.length (); i++)
    btf_asm_func_type (funcs[i]);
}

void
btf_output (const char * filename)
{
  if (btf_debug_info_level == BTFINFO_LEVEL_NONE)
    return;

  ctf_container_ref tu_ctfc = ctf_get_tu_ctfc ();

  init_btf_sections ();

  ctf_add_cuname (tu_ctfc, filename);

  btf_preprocess (tu_ctfc);

  output_btf_header (tu_ctfc);
  output_btf_types (tu_ctfc);
  output_btf_vars (tu_ctfc);
  output_btf_func_types ();
  output_btf_strs (tu_ctfc);
}

void
btf_finalize (void)
{
  btf_info_section = NULL;

  /* Clear preprocessing state.  */
  holes.release ();
  voids.release ();
  funcs.release ();

  ctf_container_ref tu_ctfc = ctf_get_tu_ctfc ();
  delete_ctf_container (tu_ctfc);
  tu_ctfc = NULL;
}

#include "gt-btfout.h"
