/* Functions to create and update CTF from GCC.
   Copyright (C) 2019 Free Software Foundation, Inc.

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

/* Create CTF types.  The code is mostly adapted from libctf.

   These functions perform the task of adding CTF types to the CTF container.
   No de-duplication is done by them; the onus is on the calling function to do
   so.  The caller must first do a lookup via ctf_dtd_lookup or
   ctf_dvd_lookup, as applicable, to ascertain that the CTF type or the CTF
   variable respectively does not already exist, and then add it.  */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "ctfout.h"

void
ctf_dtd_insert (ctf_container_ref ctfc, ctf_dtdef_ref dtd)
{
  ctf_dtdef_ref entry = dtd;
  bool existed = ctfc->ctfc_types->put (entry, dtd);
  /* Duplicate CTF type records not expected to be inserted.  And dtd_decl
     cannot be NULL.  */
  gcc_assert (dtd->dtd_decl != NULL && !existed);
}

/* Lookup CTF type given a tree type or decl node.  dtd_key_flags are not
   necessary for lookup in most cases, because they are needed only for CTF
   types with no corresponding tree type or decl to begin with.  */

ctf_dtdef_ref
ctf_dtd_lookup (const ctf_container_ref ctfc, const tree type)
{
  return ctf_dtd_lookup_with_flags (ctfc, type, 0);
}

/* Lookup CTF type given a tree type or decl node and key_flags.  */

ctf_dtdef_ref
ctf_dtd_lookup_with_flags (const ctf_container_ref ctfc, const tree type,
			   const unsigned int key_flags)
{
  ctf_dtdef_ref * slot;

  ctf_dtdef_t entry;
  entry.dtd_key.dtdk_key_decl = type;
  entry.dtd_key.dtdk_key_flags = key_flags;

  slot = ctfc->ctfc_types->get (&entry);

  if (slot)
    return (ctf_dtdef_ref) (*slot);

  return NULL;
}

void
ctf_dvd_insert (ctf_container_ref ctfc, ctf_dvdef_ref dvd)
{
  bool existed = ctfc->ctfc_vars->put (dvd->dvd_decl, dvd);
  /* Duplicate variable records not expected to be inserted.  And dvd_decl
     cannot be NULL.  */
  gcc_assert (dvd->dvd_decl != NULL && !existed);
}

/* Lookup CTF variable given a decl node.  */

ctf_dvdef_ref
ctf_dvd_lookup (const ctf_container_ref ctfc, const tree decl)
{
  ctf_dvdef_ref * slot;

  slot = ctfc->ctfc_vars->get (decl);

  if (slot)
    return (ctf_dvdef_ref) (*slot);

  return NULL;
}

static ctf_id_t
ctf_add_generic (ctf_container_ref ctfc, uint32_t flag, const char * name,
		 ctf_dtdef_ref * rp, tree treetype, uint32_t key_flags)
{
  ctf_dtdef_ref dtd;
  ctf_id_t type;

  gcc_assert (flag == CTF_ADD_NONROOT || flag == CTF_ADD_ROOT);

  dtd = ggc_cleared_alloc<ctf_dtdef_t> ();

  type = ctfc->ctfc_nextid++;
  gcc_assert (type < CTF_MAX_TYPE); /* CTF type ID overflow.  */

  /* Buffer the strings in the CTF string table.  */
  dtd->dtd_name = ctf_add_string (ctfc, name, &(dtd->dtd_data.ctti_name));
  dtd->dtd_type = type;
  dtd->dtd_key.dtdk_key_decl = treetype;
  dtd->dtd_key.dtdk_key_flags = key_flags;

  if ((name != NULL) && strcmp (name, ""))
    ctfc->ctfc_strlen += strlen (name) + 1;

  ctf_dtd_insert (ctfc, dtd);

  *rp = dtd;
  return type;
}

static ctf_id_t
ctf_add_encoded (ctf_container_ref ctfc, uint32_t flag, const char * name,
		 const ctf_encoding_t * ep, uint32_t kind, tree treetype)
{
  ctf_dtdef_ref dtd;
  ctf_id_t type;

  type = ctf_add_generic (ctfc, flag, name, &dtd, treetype, 0);

  dtd->dtd_data.ctti_info = CTF_TYPE_INFO (kind, flag, 0);

  uint32_t roundup_nbytes = (ROUND_UP (ep->cte_bits, BITS_PER_UNIT)
				    / BITS_PER_UNIT);

  /* FIXME, stay close to what libctf does.  But by getting next power of two,
     aren't we conveying less precise information.  E.g. floating point mode
     XF has a size of 12 bytes.  */
  dtd->dtd_data.ctti_size = roundup_nbytes ? (1 << ceil_log2 (roundup_nbytes))
			   : roundup_nbytes;
  dtd->dtd_u.dtu_enc = *ep;

  ctfc->ctfc_num_stypes++;

  return type;
}

static ctf_id_t
ctf_add_reftype (ctf_container_ref ctfc, uint32_t flag, ctf_id_t ref,
		 uint32_t kind, tree treetype, uint32_t cvrint)
{
  ctf_dtdef_ref dtd;
  ctf_id_t type;
  uint32_t key_flags = 0;

  /* dtd_key_flags are set only for const, volatile and restrict.  */
  if (cvrint && (kind == CTF_K_VOLATILE || kind == CTF_K_CONST
		  || kind == CTF_K_RESTRICT))
    key_flags = kind;

  gcc_assert (ref <= CTF_MAX_TYPE);

  type = ctf_add_generic (ctfc, flag, NULL, &dtd, treetype, key_flags);
  dtd->dtd_data.ctti_info = CTF_TYPE_INFO (kind, flag, 0);
  /* Caller of this API must guarantee that a CTF type with id = ref already
     exists.  This will also be validated for us at link-time.  */
  dtd->dtd_data.ctti_type = (uint32_t) ref;

  ctfc->ctfc_num_stypes++;

  return type;
}

ctf_id_t
ctf_add_forward (ctf_container_ref ctfc, uint32_t flag, const char * name,
		 uint32_t kind, tree treetype)
{
  ctf_dtdef_ref dtd;
  ctf_id_t type = 0;

  gcc_assert (!TYPE_SIZE (treetype));
  /* CTF forward types are inserted with key_flags set to 1 to distinguish them
     from the actual struct/union/union with a non-zero size which may be added
     later (the latter will be added with key_flags set to 0).  */
  uint32_t key_flags = (TYPE_SIZE (treetype) == NULL);

  type = ctf_add_generic (ctfc, flag, name, &dtd, treetype, key_flags);

  dtd->dtd_data.ctti_info = CTF_TYPE_INFO (CTF_K_FORWARD, flag, 0);
  dtd->dtd_data.ctti_type = kind;

  ctfc->ctfc_num_stypes++;

  return type;
}

ctf_id_t
ctf_add_typedef (ctf_container_ref ctfc, uint32_t flag, const char * name,
		 ctf_id_t ref, tree treetype)
{
  ctf_dtdef_ref dtd;
  ctf_id_t type;

  gcc_assert (ref <= CTF_MAX_TYPE);

  type = ctf_add_generic (ctfc, flag, name, &dtd, treetype, 0);
  dtd->dtd_data.ctti_info = CTF_TYPE_INFO (CTF_K_TYPEDEF, flag, 0);
  /* Caller of this API must guarantee that a CTF type with id = ref already
     exists.  This will also be validated for us at link-time.  */
  dtd->dtd_data.ctti_type = (uint32_t) ref;

  gcc_assert (dtd->dtd_type != dtd->dtd_data.ctti_type);

  ctfc->ctfc_num_stypes++;

  return type;
}

ctf_id_t
ctf_add_slice (ctf_container_ref ctfc, uint32_t flag, ctf_id_t ref,
	       const ctf_encoding_t * ep, tree treetype)
{
  ctf_dtdef_ref dtd;
  ctf_id_t type;
  uint32_t roundup_nbytes;

  gcc_assert ((ep->cte_bits <= 255) && (ep->cte_offset <= 255));

  gcc_assert (ref <= CTF_MAX_TYPE);

  type = ctf_add_generic (ctfc, flag, NULL, &dtd, treetype, 0);

  dtd->dtd_data.ctti_info = CTF_TYPE_INFO (CTF_K_SLICE, flag, 0);

  roundup_nbytes = (ROUND_UP (ep->cte_bits, BITS_PER_UNIT) / BITS_PER_UNIT);
  /* FIXME, stay close to what libctf does.  But by getting next power of two,
     aren't we conveying less precise information, especially for bitfields.
     For example, cte_bits = 33, roundup_nbytes = 5, ctti_size = 8 in the
     implementation below.  */
  dtd->dtd_data.ctti_size = roundup_nbytes ? (1 << ceil_log2 (roundup_nbytes))
					   : 0;

  /* Caller of this API must guarantee that a CTF type with id = ref already
     exists.  This will also be validated for us at link-time.  */
  dtd->dtd_u.dtu_slice.cts_type = (uint32_t) ref;
  dtd->dtd_u.dtu_slice.cts_bits = ep->cte_bits;
  dtd->dtd_u.dtu_slice.cts_offset = ep->cte_offset;

  ctfc->ctfc_num_stypes++;

  return type;
}

ctf_id_t
ctf_add_volatile (ctf_container_ref ctfc, uint32_t flag, ctf_id_t ref,
		  tree type, uint32_t cvrint)
{
  return (ctf_add_reftype (ctfc, flag, ref, CTF_K_VOLATILE, type, cvrint));
}

ctf_id_t
ctf_add_const (ctf_container_ref ctfc, uint32_t flag, ctf_id_t ref,
	       tree type, uint32_t cvrint)
{
  return (ctf_add_reftype (ctfc, flag, ref, CTF_K_CONST, type, cvrint));
}

ctf_id_t
ctf_add_restrict (ctf_container_ref ctfc, uint32_t flag, ctf_id_t ref,
		  tree type, uint32_t cvrint)
{
  return (ctf_add_reftype (ctfc, flag, ref, CTF_K_RESTRICT, type, cvrint));
}

ctf_id_t
ctf_add_float (ctf_container_ref ctfc, uint32_t flag,
	       const char * name, const ctf_encoding_t * ep, tree type)
{
  return (ctf_add_encoded (ctfc, flag, name, ep, CTF_K_FLOAT, type));
}

ctf_id_t
ctf_add_integer (ctf_container_ref ctfc, uint32_t flag,
		 const char * name, const ctf_encoding_t * ep, tree type)
{
  return (ctf_add_encoded (ctfc, flag, name, ep, CTF_K_INTEGER, type));
}

ctf_id_t
ctf_add_pointer (ctf_container_ref ctfc, uint32_t flag, ctf_id_t ref,
		 tree type)
{
  return (ctf_add_reftype (ctfc, flag, ref, CTF_K_POINTER, type, 0));
}

ctf_id_t
ctf_add_array (ctf_container_ref ctfc, uint32_t flag, const ctf_arinfo_t * arp,
	       tree treetype)
{
  ctf_dtdef_ref dtd;
  ctf_id_t type;

  gcc_assert (arp);

  /* Caller of this API must make sure CTF type for arp->ctr_contents and
     arp->ctr_index are already added.  This will also be validated for us at
     link-time.  */

  type = ctf_add_generic (ctfc, flag, NULL, &dtd, treetype, 0);

  dtd->dtd_data.ctti_info = CTF_TYPE_INFO (CTF_K_ARRAY, flag, 0);
  dtd->dtd_data.ctti_size = 0;
  dtd->dtd_u.dtu_arr = *arp;

  ctfc->ctfc_num_stypes++;

  return type;
}

ctf_id_t
ctf_add_enum (ctf_container_ref ctfc, uint32_t flag, const char * name,
	     HOST_WIDE_INT size, tree enum_type)
{
  ctf_dtdef_ref dtd;
  ctf_id_t type;

  /* In the compiler, no need to handle the case of promoting forwards to
     enums.  This comment is simply to note a divergence from libctf.  */

  /* The compiler does, however, update any previously existing forward types
     to non-root.  CTF does not allow existence of two root types with the same
     name.  */
  ctf_dtdef_ref enum_fwd_type = ctf_dtd_lookup_with_flags (ctfc, enum_type, 1);
  if (enum_fwd_type)
    {
      enum_fwd_type->dtd_data.ctti_info
	= CTF_TYPE_INFO (CTF_K_FORWARD, CTF_ADD_NONROOT, 0);
    }

  type = ctf_add_generic (ctfc, flag, name, &dtd, enum_type, 0);

  dtd->dtd_data.ctti_info = CTF_TYPE_INFO (CTF_K_ENUM, flag, 0);

  /* Size in bytes should always fit, of course.
     TBD WARN - warn instead?  */
  gcc_assert (size <= CTF_MAX_SIZE);

  dtd->dtd_data.ctti_size = size;

  ctfc->ctfc_num_stypes++;

  return type;
}

int
ctf_add_enumerator (ctf_container_ref ctfc, ctf_id_t enid, const char * name,
		    HOST_WIDE_INT value, tree enum_type)
{
  ctf_dmdef_t * dmd;
  uint32_t kind, vlen, root;

  /* Callers of this API must make sure that CTF_K_ENUM with enid has been
     addded.  This will also be validated for us at link-time.  */
  ctf_dtdef_ref dtd = ctf_dtd_lookup (ctfc, enum_type);
  gcc_assert (dtd);
  gcc_assert (dtd->dtd_type == enid);
  gcc_assert (name);

  kind = CTF_V2_INFO_KIND (dtd->dtd_data.ctti_info);
  root = CTF_V2_INFO_ISROOT (dtd->dtd_data.ctti_info);
  vlen = CTF_V2_INFO_VLEN (dtd->dtd_data.ctti_info);

  gcc_assert (kind == CTF_K_ENUM && vlen < CTF_MAX_VLEN);

  /* Enum value is of type HOST_WIDE_INT in the compiler, dmd_value is int32_t
     on the other hand.  Check bounds and skip adding this enum value if out of
     bounds.  */
  if ((value > INT_MAX) || (value < INT_MIN))
    {
      /* FIXME - Note this TBD_CTF_REPRESENTATION_LIMIT.  */
      return (1);
    }

  dmd = ggc_cleared_alloc<ctf_dmdef_t> ();

  /* Buffer the strings in the CTF string table.  */
  dmd->dmd_name = ctf_add_string (ctfc, name, &(dmd->dmd_name_offset));
  dmd->dmd_type = CTF_NULL_TYPEID;
  dmd->dmd_offset = 0;

  dmd->dmd_value = value;

  dtd->dtd_data.ctti_info = CTF_TYPE_INFO (kind, root, vlen + 1);
  ctf_dmd_list_append (&dtd->dtd_u.dtu_members, dmd);

  if ((name != NULL) && strcmp (name, ""))
    ctfc->ctfc_strlen += strlen (name) + 1;

  return (0);
}

int
ctf_add_member_offset (ctf_container_ref ctfc, tree sou, const char * name,
		       ctf_id_t type, unsigned long bit_offset)
{
  ctf_dtdef_ref dtd = ctf_dtd_lookup (ctfc, sou);
  ctf_dmdef_t * dmd;

  uint32_t kind, vlen, root;

  /* The type of the member being added must already exist.  */
  gcc_assert (dtd);

  kind = CTF_V2_INFO_KIND (dtd->dtd_data.ctti_info);
  root = CTF_V2_INFO_ISROOT (dtd->dtd_data.ctti_info);
  vlen = CTF_V2_INFO_VLEN (dtd->dtd_data.ctti_info);

  gcc_assert (kind == CTF_K_STRUCT || kind == CTF_K_UNION);
  gcc_assert (vlen < CTF_MAX_VLEN);

#if 0
  /* Check duplicate members with the same name.  May be a useful check if
     members of anonymous truct or union are folded into the parent struct (if
     exists); See a pending TBD in gen_ctf_sou_type for more info.  */
  if (name != NULL)
    {
      for (dmd = dtd->dtd_u.dtu_members;
	   dmd != NULL; dmd = (ctf_dmdef_t *) ctf_dmd_list_next (dmd))
	{
	  if (dmd->dmd_name != NULL)
	    gcc_assert (strcmp (dmd->dmd_name, name) != 0);
	}
    }
#endif

  dmd = ggc_cleared_alloc<ctf_dmdef_t> ();

  /* Buffer the strings in the CTF string table.  */
  dmd->dmd_name = ctf_add_string (ctfc, name, &(dmd->dmd_name_offset));
  dmd->dmd_type = type;
  dmd->dmd_value = -1;

  if (kind == CTF_K_STRUCT && vlen != 0)
    dmd->dmd_offset = bit_offset;
  else
    dmd->dmd_offset = 0;

  dtd->dtd_data.ctti_info = CTF_TYPE_INFO (kind, root, vlen + 1);
  ctf_dmd_list_append (&dtd->dtd_u.dtu_members, dmd);

  if ((name != NULL) && strcmp (name, ""))
    ctfc->ctfc_strlen += strlen (name) + 1;

  return 0;
}

int
ctf_add_variable (ctf_container_ref ctfc, const char * name, ctf_id_t ref,
		  tree decl)
{
  ctf_dvdef_ref dvd;

  gcc_assert (name);

  if (name != NULL)
    {
      dvd = ggc_cleared_alloc<ctf_dvdef_t> ();
      /* Buffer the strings in the CTF string table.  */
      dvd->dvd_name = ctf_add_string (ctfc, name, &(dvd->dvd_name_offset));
      dvd->dvd_type = ref;
      dvd->dvd_decl = decl;
      ctf_dvd_insert (ctfc, dvd);

      if (strcmp (name, ""))
	ctfc->ctfc_strlen += strlen (name) + 1;
    }

  return 0;
}

ctf_id_t
ctf_add_function (ctf_container_ref ctfc, uint32_t flag, const char * name,
		  const ctf_funcinfo_t * ctc, ctf_func_arg_t * argv,
		  tree func_decl_or_type)
{
  ctf_dtdef_ref dtd;
  ctf_id_t type;
  uint32_t vlen;

  gcc_assert (ctc);
  if (ctc->ctc_argc)
    gcc_assert (argv);

  vlen = ctc->ctc_argc;

  /* Caller must make sure CTF types for ctc->ctc_return and function
     arguements are already added.  */

  gcc_assert (vlen <= CTF_MAX_VLEN);

  type = ctf_add_generic (ctfc, flag, name, &dtd, func_decl_or_type, 0);

  dtd->dtd_data.ctti_info = CTF_TYPE_INFO (CTF_K_FUNCTION, flag, vlen);
  dtd->dtd_data.ctti_type = (uint32_t) ctc->ctc_return;

  dtd->dtd_u.dtu_argv = argv;

  ctfc->ctfc_num_stypes++;

  return type;
}

ctf_id_t
ctf_add_sou (ctf_container_ref ctfc, uint32_t flag, const char * name,
	     uint32_t kind, size_t size, tree treetype)
{
  ctf_dtdef_ref dtd;
  ctf_id_t type = 0;

  gcc_assert ((kind == CTF_K_STRUCT) || (kind == CTF_K_UNION));

  /* In the compiler, no need to handle the case of promoting forwards to
     structs.  This comment is simply to note a divergence from libctf.  */

  /* The compiler does, however, update any previously existing forward types
     to non-root.  CTF does not allow existence of two root types with the same
     name.  */
  ctf_dtdef_ref sou_fwd_type = ctf_dtd_lookup_with_flags (ctfc, treetype, 1);
  if (sou_fwd_type)
    {
      sou_fwd_type->dtd_data.ctti_info
	= CTF_TYPE_INFO (CTF_K_FORWARD, CTF_ADD_NONROOT, 0);
    }

  type = ctf_add_generic (ctfc, flag, name, &dtd, treetype, 0);

  dtd->dtd_data.ctti_info = CTF_TYPE_INFO (kind, flag, 0);

  if (size > CTF_MAX_SIZE)
    {
      dtd->dtd_data.ctti_size = CTF_LSIZE_SENT;
      dtd->dtd_data.ctti_lsizehi = CTF_SIZE_TO_LSIZE_HI (size);
      dtd->dtd_data.ctti_lsizelo = CTF_SIZE_TO_LSIZE_LO (size);
      ctfc->ctfc_num_types++;
    }
  else
    {
      dtd->dtd_data.ctti_size = (uint32_t) size;
      ctfc->ctfc_num_stypes++;
    }

  return type;
}
