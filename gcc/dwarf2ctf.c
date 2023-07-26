/* Generate CTF types and objects from the GCC DWARF.
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

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "target.h"
#include "dwarf2out.h"
#include "dwarf2out.h"

#include "dwarf2ctf.h"
#include "ctfc.h"

/* Forward declarations for some routines defined in this file.  */

static ctf_id_t
gen_ctf_type (ctf_container_ref, dw_die_ref);

/* All the DIE structures we handle come from the DWARF information
   generated by GCC.  However, there are two situations where we need
   to create our own created DIE structures because GCC doesn't
   provide them.

   The DWARF spec suggests using a DIE with DW_TAG_unspecified_type
   and name "void" in order to denote the void type.  But GCC doesn't
   follow this advice.  Still we need a DIE to act as a key for void
   types, we use ctf_void_die.

   Also, if a subrange type corresponding to an array index does not
   specify a type then we assume it is `int'.

   Finally, for types unrepresentable in CTF, we need a DIE to anchor
   them to a CTF type of kind unknown.

   The variables below are initialized in ctf_debug_init and hold
   references to the proper DIEs.  */

static GTY (()) dw_die_ref ctf_void_die;
static GTY (()) dw_die_ref ctf_array_index_die;
static GTY (()) dw_die_ref ctf_unknown_die;

/* Some DIEs have a type description attribute, stored in a DW_AT_type
   attribute.  However, GCC generates no attribute to signify a `void'
   type.

   This can happen in many contexts (return type of a function,
   pointed or qualified type, etc) so we use the `ctf_get_AT_type'
   function below abstracts this.  */

static dw_die_ref
ctf_get_AT_type (dw_die_ref die)
{
  dw_die_ref type_die = get_AT_ref (die, DW_AT_type);
  return (type_die ? type_die : ctf_void_die);
}

/* Some data member DIEs have location specified as a DWARF expression
   (specifically in DWARF2).  Luckily, the expression is a simple
   DW_OP_plus_uconst with one operand set to zero.

   Sometimes the data member location may also be negative.  In yet some other
   cases (specifically union data members), the data member location is
   non-existent.  Handle all these scenarios here to abstract this.  */

static HOST_WIDE_INT
ctf_get_AT_data_member_location (dw_die_ref die)
{
  HOST_WIDE_INT field_location = 0;
  dw_attr_node * attr;

  /* The field location (in bits) can be determined from
     either a DW_AT_data_member_location attribute or a
     DW_AT_data_bit_offset attribute.  */
  if (get_AT (die, DW_AT_data_bit_offset))
    field_location = get_AT_unsigned (die, DW_AT_data_bit_offset);
  else
    {
      attr = get_AT (die, DW_AT_data_member_location);
      if (attr && AT_class (attr) == dw_val_class_loc)
	{
	  dw_loc_descr_ref descr = AT_loc (attr);
	  /* Operand 2 must be zero; the structure is assumed to be on the
	     stack in DWARF2.  */
	  gcc_assert (!descr->dw_loc_oprnd2.v.val_unsigned);
	  gcc_assert (descr->dw_loc_oprnd2.val_class
		      == dw_val_class_unsigned_const);
	  field_location = descr->dw_loc_oprnd1.v.val_unsigned * 8;
	}
      else
	{
	  attr = get_AT (die, DW_AT_data_member_location);
	  if (attr && AT_class (attr) == dw_val_class_const)
	    field_location = AT_int (attr) * 8;
	  else
	    field_location = (get_AT_unsigned (die,
					   DW_AT_data_member_location)
			      * 8);
	}
    }

  return field_location;
}

/* CTF Types' and CTF Variables' Location Information.  CTF section does not
   emit location information, this is provided for BTF CO-RE use-cases.  These
   functions fetch information from DWARF Die directly, as such the location
   information is not buffered in the CTF container.  */

const char *
ctf_get_die_loc_file (dw_die_ref die)
{
  if (!die)
    return NULL;

  struct dwarf_file_data * file;
  file = get_AT_file (die, DW_AT_decl_file);
  if (!file)
    return NULL;

  return file->filename;
}

unsigned int
ctf_get_die_loc_line (dw_die_ref die)
{
  if (!die)
    return 0;

  return get_AT_unsigned (die, DW_AT_decl_line);
}

unsigned int
ctf_get_die_loc_col (dw_die_ref die)
{
  if (!die)
    return 0;

  return get_AT_unsigned (die, DW_AT_decl_column);
}

/* Generate CTF for the void type.  */

static ctf_id_t
gen_ctf_void_type (ctf_container_ref ctfc)
{
  ctf_encoding_t ctf_encoding = {0, 0, 0};

  /* In CTF the void type is encoded as a 0-byte signed integer
     type.  */

  ctf_encoding.cte_bits = 0;
  ctf_encoding.cte_format = CTF_INT_SIGNED;

  gcc_assert (ctf_void_die != NULL);
  return ctf_add_integer (ctfc, CTF_ADD_ROOT, "void",
			  &ctf_encoding, ctf_void_die);
}

/* Generate CTF type of unknown kind.  */

static ctf_id_t
gen_ctf_unknown_type (ctf_container_ref ctfc)
{
  ctf_id_t unknown_type_id;

  /* In CTF, the unknown type is encoded as a 0 byte sized type with kind
     CTF_K_UNKNOWN.  Create an encoding object merely to reuse the underlying
     ctf_add_encoded interface; the CTF encoding object is not 'used' any more
     than just the generation of size from.  */
  ctf_encoding_t ctf_encoding = {0, 0, 0};

  gcc_assert (ctf_unknown_die != NULL);
  /* Type de-duplication.  */
  if (!ctf_type_exists (ctfc, ctf_unknown_die, &unknown_type_id))
    unknown_type_id = ctf_add_unknown (ctfc, CTF_ADD_ROOT, "unknown",
				       &ctf_encoding, ctf_unknown_die);

  return unknown_type_id;
}

/* Sizes of entities can be given in bytes or bits.  This function
   abstracts this by returning the size in bits of the given entity.
   If no DW_AT_byte_size nor DW_AT_bit_size are defined, this function
   returns 0.  */

static uint32_t
ctf_die_bitsize (dw_die_ref die)
{
  dw_attr_node *attr_byte_size = get_AT (die, DW_AT_byte_size);
  dw_attr_node *attr_bit_size = get_AT (die, DW_AT_bit_size);

  if (attr_bit_size)
    return AT_unsigned (attr_bit_size);
  else if (attr_byte_size)
    return (AT_unsigned (attr_byte_size) * 8);
  else
    return 0;
}

/* Generate CTF for base type (integer, boolean, real, fixed point and complex).
   Important: the caller of this API must make sure that duplicate types are
   not added.  */

static ctf_id_t
gen_ctf_base_type (ctf_container_ref ctfc, dw_die_ref type)
{
  ctf_id_t type_id = CTF_NULL_TYPEID;

  ctf_encoding_t ctf_encoding = {0, 0, 0};

  unsigned int encoding = get_AT_unsigned (type, DW_AT_encoding);
  unsigned int bit_size = ctf_die_bitsize (type);
  const char * name_string = get_AT_string (type, DW_AT_name);

  switch (encoding)
    {
    case DW_ATE_void:

      ctf_encoding.cte_format = CTF_INT_SIGNED;
      ctf_encoding.cte_bits = 0;

      gcc_assert (name_string);
      type_id = ctf_add_integer (ctfc, CTF_ADD_ROOT, name_string,
				 &ctf_encoding, type);

      break;
    case DW_ATE_boolean:

      ctf_encoding.cte_format = CTF_INT_BOOL;
      ctf_encoding.cte_bits = bit_size;

      gcc_assert (name_string);
      type_id = ctf_add_integer (ctfc, CTF_ADD_ROOT, name_string,
				 &ctf_encoding, type);
      break;
    case DW_ATE_float:
      {
	unsigned int float_bit_size
	  = tree_to_uhwi (TYPE_SIZE (float_type_node));
	unsigned int double_bit_size
	  = tree_to_uhwi (TYPE_SIZE (double_type_node));
	unsigned int long_double_bit_size
	  = tree_to_uhwi (TYPE_SIZE (long_double_type_node));

	if (bit_size == float_bit_size)
	  ctf_encoding.cte_format = CTF_FP_SINGLE;
	else if (bit_size == double_bit_size)
	  ctf_encoding.cte_format = CTF_FP_DOUBLE;
	else if (bit_size == long_double_bit_size)
	  ctf_encoding.cte_format = CTF_FP_LDOUBLE;
	else
	  /* CTF does not have representation for other types.  Skip them.  */
	  break;

	ctf_encoding.cte_bits = bit_size;
	type_id = ctf_add_float (ctfc, CTF_ADD_ROOT, name_string,
				 &ctf_encoding, type);

	break;
      }
    case DW_ATE_signed_char:
      /* FALLTHROUGH */
    case DW_ATE_unsigned_char:
      /* FALLTHROUGH */
    case DW_ATE_signed:
      /* FALLTHROUGH */
    case DW_ATE_unsigned:

      if (encoding == DW_ATE_signed_char
	  || encoding == DW_ATE_unsigned_char)
	ctf_encoding.cte_format |= CTF_INT_CHAR;

      if (encoding == DW_ATE_signed
	  || encoding == DW_ATE_signed_char)
	ctf_encoding.cte_format |= CTF_INT_SIGNED;

      ctf_encoding.cte_bits = bit_size;
      type_id = ctf_add_integer (ctfc, CTF_ADD_ROOT, name_string,
				 &ctf_encoding, type);
      break;

    case DW_ATE_complex_float:
      {
	unsigned int float_bit_size
	  = tree_to_uhwi (TYPE_SIZE (float_type_node));
	unsigned int double_bit_size
	  = tree_to_uhwi (TYPE_SIZE (double_type_node));
	unsigned int long_double_bit_size
	  = tree_to_uhwi (TYPE_SIZE (long_double_type_node));

	if (bit_size == float_bit_size * 2)
	  ctf_encoding.cte_format = CTF_FP_CPLX;
	else if (bit_size == double_bit_size * 2)
	  ctf_encoding.cte_format = CTF_FP_DCPLX;
	else if (bit_size == long_double_bit_size * 2)
	    ctf_encoding.cte_format = CTF_FP_LDCPLX;
	else
	  /* CTF does not have representation for other types.  Skip them.  */
	  break;

	ctf_encoding.cte_bits = bit_size;
	type_id = ctf_add_float (ctfc, CTF_ADD_ROOT, name_string,
				 &ctf_encoding, type);
	break;
      }
    default:
      /* Ignore.  */
      break;
    }

  return type_id;
}

/* Generate CTF for a pointer type.  */

static ctf_id_t
gen_ctf_pointer_type (ctf_container_ref ctfc, dw_die_ref ptr_type)
{
  ctf_id_t type_id = CTF_NULL_TYPEID;
  ctf_id_t ptr_type_id = CTF_NULL_TYPEID;
  dw_die_ref pointed_type_die = ctf_get_AT_type (ptr_type);

  type_id = gen_ctf_type (ctfc, pointed_type_die);

  /* Type de-duplication.
     Consult the ctfc_types hash again before adding the CTF pointer type
     because there can be cases where a pointer type may have been added by
     the gen_ctf_type call above.  */
  if (ctf_type_exists (ctfc, ptr_type, &ptr_type_id))
    return ptr_type_id;

  ptr_type_id = ctf_add_pointer (ctfc, CTF_ADD_ROOT, type_id, ptr_type);
  return ptr_type_id;
}

/* Generate CTF for an array type.  */

static ctf_id_t
gen_ctf_array_type (ctf_container_ref ctfc, dw_die_ref array_type)
{
  dw_die_ref c;
  ctf_id_t array_elems_type_id = CTF_NULL_TYPEID;

  int vector_type_p = get_AT_flag (array_type, DW_AT_GNU_vector);
  if (vector_type_p)
    return array_elems_type_id;

  dw_die_ref array_elems_type = ctf_get_AT_type (array_type);

  /* First, register the type of the array elements if needed.  */
  array_elems_type_id = gen_ctf_type (ctfc, array_elems_type);

  /* DWARF array types pretend C supports multi-dimensional arrays.
     So for the type int[N][M], the array type DIE contains two
     subrange_type children, the first iwth upper bound N-1 and the
     second with upper bound M-1.

     CTF, on the other hand, just encodes each array type in its own
     array type CTF struct.  Therefore we have to iterate on the
     children and create all the needed types.  */

  c = dw_get_die_child (array_type);
  gcc_assert (c);
  do
    {
      ctf_arinfo_t arinfo;
      dw_die_ref array_index_type;
      uint32_t array_num_elements;

      c = dw_get_die_sib (c);

      if (dw_get_die_tag (c) == DW_TAG_subrange_type)
	{
	  dw_attr_node *upper_bound_at;

	  array_index_type = ctf_get_AT_type (c);

	  /* When DW_AT_upper_bound is used to specify the size of an
	     array in DWARF, it is usually an unsigned constant
	     specifying the upper bound index of the array.  However,
	     for unsized arrays, such as foo[] or bar[0],
	     DW_AT_upper_bound is a signed integer constant
	     instead.  */

	  upper_bound_at = get_AT (c, DW_AT_upper_bound);
	  if (upper_bound_at
	      && AT_class (upper_bound_at) == dw_val_class_unsigned_const)
	    /* This is the upper bound index.  */
	    array_num_elements = get_AT_unsigned (c, DW_AT_upper_bound) + 1;
	  else if (get_AT (c, DW_AT_count))
	    array_num_elements = get_AT_unsigned (c, DW_AT_count);
	  else
	    {
	      /* This is a VLA of some kind.  */
	      array_num_elements = 0;
	    }
	}
      else if (dw_get_die_tag (c) == DW_TAG_enumeration_type)
	{
	  array_index_type = 0;
	  array_num_elements = 0;
	  /* XXX writeme. */
	  gcc_assert (1);
	}
      else
	gcc_unreachable ();

      /* Ok, mount and register the array type.  Note how the array
	 type we register here is the type of the elements in
	 subsequent "dimensions", if there are any.  */

      arinfo.ctr_nelems = array_num_elements;
      if (array_index_type)
	arinfo.ctr_index = gen_ctf_type (ctfc, array_index_type);
      else
	arinfo.ctr_index = gen_ctf_type (ctfc, ctf_array_index_die);

      arinfo.ctr_contents = array_elems_type_id;
      if (!ctf_type_exists (ctfc, c, &array_elems_type_id))
	array_elems_type_id = ctf_add_array (ctfc, CTF_ADD_ROOT, &arinfo,
					     c);
    }
  while (c != dw_get_die_child (array_type));

#if 0
  /* Type de-duplication.
     Consult the ctfc_types hash again before adding the CTF array type because
     there can be cases where an array_type type may have been added by the
     gen_ctf_type call above.  */
  if (!ctf_type_exists (ctfc, array_type, &type_id))
    type_id = ctf_add_array (ctfc, CTF_ADD_ROOT, &arinfo, array_type);
#endif

  return array_elems_type_id;
}

/* Generate CTF for a typedef.  */

static ctf_id_t
gen_ctf_typedef (ctf_container_ref ctfc, dw_die_ref tdef)
{
  ctf_id_t tdef_type_id, tid;
  const char *tdef_name = get_AT_string (tdef, DW_AT_name);
  dw_die_ref tdef_type = ctf_get_AT_type (tdef);

  tid = gen_ctf_type (ctfc, tdef_type);

  /* Type de-duplication.
     This is necessary because the ctf for the typedef may have been already
     added due to the gen_ctf_type call above.  */
  if (!ctf_type_exists (ctfc, tdef, &tdef_type_id))
  {
    tdef_type_id = ctf_add_typedef (ctfc, CTF_ADD_ROOT,
				    tdef_name,
				    tid,
				    tdef);
  }
  return tdef_type_id;
}

/* Generate CTF for a type modifier.

   If the given DIE contains a valid C modifier (like _Atomic), which is not
   supported by CTF, then this function skips the modifier die and continues
   with the underlying type.

   For all other cases, this function returns a CTF_NULL_TYPEID;
*/

static ctf_id_t
gen_ctf_modifier_type (ctf_container_ref ctfc, dw_die_ref modifier)
{
  uint32_t kind = CTF_K_MAX;
  ctf_id_t modifier_type_id, qual_type_id;
  dw_die_ref qual_type = ctf_get_AT_type (modifier);

  switch (dw_get_die_tag (modifier))
    {
    case DW_TAG_const_type: kind = CTF_K_CONST; break;
    case DW_TAG_volatile_type: kind = CTF_K_VOLATILE; break;
    case DW_TAG_restrict_type: kind = CTF_K_RESTRICT; break;
    case DW_TAG_atomic_type: break;
    default:
      return CTF_NULL_TYPEID;
    }

  /* Register the type for which this modifier applies.  */
  qual_type_id = gen_ctf_type (ctfc, qual_type);

  /* Skip generating a CTF modifier record for _Atomic as there is no
     representation for it.  */
  if (dw_get_die_tag (modifier) == DW_TAG_atomic_type)
    return qual_type_id;

  gcc_assert (kind != CTF_K_MAX);
  /* Now register the modifier itself.  */
  if (!ctf_type_exists (ctfc, modifier, &modifier_type_id))
    modifier_type_id = ctf_add_reftype (ctfc, CTF_ADD_ROOT,
					qual_type_id, kind,
					modifier);

  return modifier_type_id;
}

/* Generate CTF for a struct type.  */

static ctf_id_t
gen_ctf_sou_type (ctf_container_ref ctfc, dw_die_ref sou, uint32_t kind)
{
  uint32_t bit_size = ctf_die_bitsize (sou);
  int declaration_p = get_AT_flag (sou, DW_AT_declaration);
  const char *sou_name = get_AT_string (sou, DW_AT_name);

  ctf_id_t sou_type_id;

  /* An incomplete structure or union type is represented in DWARF by
     a structure or union DIE that does not have a size attribute and
     that has a DW_AT_declaration attribute.  This corresponds to a
     CTF forward type with kind CTF_K_STRUCT.  */
  if (bit_size == 0 && declaration_p)
    return ctf_add_forward (ctfc, CTF_ADD_ROOT,
			    sou_name, kind, sou);

  /* This is a complete struct or union type.  Generate a CTF type for
     it if it doesn't exist already.  */
  if (!ctf_type_exists (ctfc, sou, &sou_type_id))
    sou_type_id = ctf_add_sou (ctfc, CTF_ADD_ROOT,
			       sou_name, kind, bit_size / 8,
			       sou);

  /* Now process the struct members.  */
  {
    dw_die_ref c;

    c = dw_get_die_child (sou);
    if (c)
      do
	{
	  const char *field_name;
	  dw_die_ref field_type;
	  HOST_WIDE_INT field_location;
	  ctf_id_t field_type_id;

	  c = dw_get_die_sib (c);

	  field_name = get_AT_string (c, DW_AT_name);
	  field_type = ctf_get_AT_type (c);
	  field_location = ctf_get_AT_data_member_location (c);

	  /* Generate the field type.  */
	  field_type_id = gen_ctf_type (ctfc, field_type);

	  /* If this is a bit-field, then wrap the field type
	     generated above with a CTF slice.  */
	  if (get_AT (c, DW_AT_bit_offset)
	      || get_AT (c, DW_AT_data_bit_offset))
	    {
	      dw_attr_node *attr;
	      HOST_WIDE_INT bitpos = 0;
	      HOST_WIDE_INT bitsize = ctf_die_bitsize (c);
	      HOST_WIDE_INT bit_offset;

	      /* The bit offset is given in bits and it may be
		 negative.  */
	      attr = get_AT (c, DW_AT_bit_offset);
	      if (attr)
		{
		  if (AT_class (attr) == dw_val_class_unsigned_const)
		    bit_offset = AT_unsigned (attr);
		  else
		    bit_offset = AT_int (attr);

		  if (BYTES_BIG_ENDIAN)
		    bitpos = field_location + bit_offset;
		  else
		    {
		      HOST_WIDE_INT bit_size;

		      attr = get_AT (c, DW_AT_byte_size);
		      if (attr)
			/* Explicit size given in bytes.  */
			bit_size = AT_unsigned (attr) * 8;
		      else
			/* Infer the size from the member type.  */
			bit_size = ctf_die_bitsize (field_type);

		      bitpos = (field_location
				+ bit_size
				- bit_offset
				- bitsize);
		    }
		}

	      /* In DWARF5 a data_bit_offset attribute is given with
		 the offset of the data from the beginning of the
		 struct.  Acknowledge it if present.  */
	      attr = get_AT (c, DW_AT_data_bit_offset);
	      if (attr)
		bitpos += AT_unsigned (attr);

	      field_type_id = ctf_add_slice (ctfc, CTF_ADD_NONROOT,
					     field_type_id,
					     bitpos - field_location,
					     bitsize,
					     c);
	    }

	  /* Add the field type to the struct or union type.  */
	  ctf_add_member_offset (ctfc, sou,
				 field_name,
				 field_type_id,
				 field_location);
	}
      while (c != dw_get_die_child (sou));
  }

  return sou_type_id;
}

/* Generate CTF for a function type.  */

static ctf_id_t
gen_ctf_function_type (ctf_container_ref ctfc, dw_die_ref function,
		       bool from_global_func)
{
  const char *function_name = get_AT_string (function, DW_AT_name);
  dw_die_ref return_type = ctf_get_AT_type (function);

  ctf_funcinfo_t func_info;
  uint32_t num_args = 0;

  ctf_id_t return_type_id;
  ctf_id_t function_type_id;

  /* First, add the return type.  */
  return_type_id = gen_ctf_type (ctfc, return_type);
  func_info.ctc_return = return_type_id;

  /* Type de-duplication.
     Consult the ctfc_types hash before adding the CTF function type.  */
  if (ctf_type_exists (ctfc, function, &function_type_id))
    return function_type_id;

  /* Do a first pass on the formals to determine the number of
     arguments, and whether the function type gets a varargs.  */
  {
    dw_die_ref c;

    c = dw_get_die_child (function);
    if (c)
      do
	{
	  c = dw_get_die_sib (c);

	  if (dw_get_die_tag (c) == DW_TAG_formal_parameter)
	    num_args += 1;
	  else if (dw_get_die_tag (c) == DW_TAG_unspecified_parameters)
	    {
	      func_info.ctc_flags |= CTF_FUNC_VARARG;
	      num_args += 1;
	    }
	}
      while (c != dw_get_die_child (function));
  }

  /* Note the number of typed arguments _includes_ the vararg.  */
  func_info.ctc_argc = num_args;

  /* Type de-duplication has already been performed by now.  */
  function_type_id = ctf_add_function (ctfc, CTF_ADD_ROOT,
				       function_name,
				       (const ctf_funcinfo_t *)&func_info,
				       function,
				       from_global_func);

  /* Second pass on formals: generate the CTF types corresponding to
     them and add them as CTF function args.  */
  {
    dw_die_ref c;
    unsigned int i = 0;
    const char *arg_name;
    ctf_id_t arg_type;

    c = dw_get_die_child (function);
    if (c)
      do
	{
	  c = dw_get_die_sib (c);

	  if (dw_get_die_tag (c) == DW_TAG_unspecified_parameters)
	    {
	      gcc_assert (i == num_args - 1);
	      /* Add an argument with type 0 and no name.  */
	      ctf_add_function_arg (ctfc, function, "", 0);
	    }
	  else if (dw_get_die_tag (c) == DW_TAG_formal_parameter)
	    {
	      i++;
	      arg_name = get_AT_string (c, DW_AT_name);
	      arg_type = gen_ctf_type (ctfc, ctf_get_AT_type (c));
	      /* Add the argument to the existing CTF function type.  */
	      ctf_add_function_arg (ctfc, function, arg_name, arg_type);
	    }
	  else
	    /* This is a local variable.  Ignore.  */
	    continue;
	}
      while (c != dw_get_die_child (function));
  }

  return function_type_id;
}

/* Generate CTF for an enumeration type.  */

static ctf_id_t
gen_ctf_enumeration_type (ctf_container_ref ctfc, dw_die_ref enumeration)
{
  const char *enum_name = get_AT_string (enumeration, DW_AT_name);
  unsigned int bit_size = ctf_die_bitsize (enumeration);
  int declaration_p = get_AT_flag (enumeration, DW_AT_declaration);

  ctf_id_t enumeration_type_id;

  /* If this is an incomplete enum, generate a CTF forward for it and
     be done.  */
  if (declaration_p)
    {
      gcc_assert (enum_name);
      return ctf_add_forward (ctfc, CTF_ADD_ROOT, enum_name,
			      CTF_K_ENUM, enumeration);
    }

  /* If the size the enumerators is not specified then use the size of
     the underlying type, which must be a base type.  */
  if (bit_size == 0)
    {
      dw_die_ref type = ctf_get_AT_type (enumeration);
      bit_size = ctf_die_bitsize (type);
    }

  /* Generate a CTF type for the enumeration.  */
  enumeration_type_id = ctf_add_enum (ctfc, CTF_ADD_ROOT,
				      enum_name, bit_size / 8, enumeration);

  /* Process the enumerators.  */
  {
    dw_die_ref c;

    c = dw_get_die_child (enumeration);
    if (c)
      do
	{
	  const char *enumerator_name;
	  dw_attr_node *enumerator_value;
	  HOST_WIDE_INT value_wide_int;

	  c = dw_get_die_sib (c);

	  enumerator_name = get_AT_string (c, DW_AT_name);
	  enumerator_value = get_AT (c, DW_AT_const_value);

	  /* enumerator_value can be either a signed or an unsigned
	     constant value.  */
	  if (AT_class (enumerator_value) == dw_val_class_unsigned_const
	      || (AT_class (enumerator_value)
		  == dw_val_class_unsigned_const_implicit))
	    value_wide_int = AT_unsigned (enumerator_value);
	  else
	    value_wide_int = AT_int (enumerator_value);

	  ctf_add_enumerator (ctfc, enumeration_type_id,
			      enumerator_name, value_wide_int, enumeration);
	}
      while (c != dw_get_die_child (enumeration));
  }

  return enumeration_type_id;
}

/* Add a CTF variable record for the given input DWARF DIE.  */

static void
gen_ctf_variable (ctf_container_ref ctfc, dw_die_ref die)
{
  const char *var_name = get_AT_string (die, DW_AT_name);
  dw_die_ref var_type = ctf_get_AT_type (die);
  unsigned int external_vis = get_AT_flag (die, DW_AT_external);
  ctf_id_t var_type_id;

  /* Avoid duplicates.  */
  if (ctf_dvd_lookup (ctfc, die))
    return;

  /* Add the type of the variable.  */
  var_type_id = gen_ctf_type (ctfc, var_type);

  /* Generate the new CTF variable and update global counter.  */
  (void) ctf_add_variable (ctfc, var_name, var_type_id, die, external_vis);
  ctfc->ctfc_num_global_objts += 1;
}

/* Add a CTF function record for the given input DWARF DIE.  */

static void
gen_ctf_function (ctf_container_ref ctfc, dw_die_ref die)
{
  ctf_id_t function_type_id;
  /* Type de-duplication.
     Consult the ctfc_types hash before adding the CTF function type.  */
  if (ctf_type_exists (ctfc, die, &function_type_id))
    return;

  /* Add the type of the function and update the global functions
     counter.  Note that DWARF encodes function types in both
     DW_TAG_subroutine_type and DW_TAG_subprogram in exactly the same
     way.  */
  (void) gen_ctf_function_type (ctfc, die, true /* from_global_func */);
  ctfc->ctfc_num_global_funcs += 1;
}

/* Add CTF type record(s) for the given input DWARF DIE and return its type id.

   If there is already a CTF type corresponding to the given DIE, then
   this function returns the type id of the existing type.

   If the given DIE is not recognized as a type, then this function
   returns CTF_NULL_TYPEID.  */

static ctf_id_t
gen_ctf_type (ctf_container_ref ctfc, dw_die_ref die)
{
  ctf_id_t type_id;
  int unrecog_die = false;

  if (ctf_type_exists (ctfc, die, &type_id))
    return type_id;

  switch (dw_get_die_tag (die))
    {
    case DW_TAG_base_type:
      type_id = gen_ctf_base_type (ctfc, die);
      break;
    case DW_TAG_pointer_type:
      type_id = gen_ctf_pointer_type (ctfc, die);
      break;
    case DW_TAG_typedef:
      type_id = gen_ctf_typedef (ctfc, die);
      break;
    case DW_TAG_array_type:
      type_id = gen_ctf_array_type (ctfc, die);
      break;
    case DW_TAG_structure_type:
      type_id = gen_ctf_sou_type (ctfc, die, CTF_K_STRUCT);
      break;
    case DW_TAG_union_type:
      type_id = gen_ctf_sou_type (ctfc, die, CTF_K_UNION);
      break;
    case DW_TAG_subroutine_type:
      type_id = gen_ctf_function_type (ctfc, die,
				       false /* from_global_func */);
      break;
    case DW_TAG_enumeration_type:
      type_id = gen_ctf_enumeration_type (ctfc, die);
      break;
    case DW_TAG_atomic_type:
      /* FALLTHROUGH */
    case DW_TAG_const_type:
      /* FALLTHROUGH */
    case DW_TAG_restrict_type:
      /* FALLTHROUGH */
    case DW_TAG_volatile_type:
      type_id = gen_ctf_modifier_type (ctfc, die);
      break;
    case DW_TAG_unspecified_type:
      {
	const char *name = get_AT_string (die, DW_AT_name);

	if (name && strcmp (name, "void") == 0)
	  type_id = gen_ctf_void_type (ctfc);
	else
	  type_id = CTF_NULL_TYPEID;

	break;
      }
    case DW_TAG_reference_type:
      type_id = CTF_NULL_TYPEID;
      break;
    default:
      /* Unrecognized DIE.  */
      unrecog_die = true;
      type_id = CTF_NULL_TYPEID;
      break;
    }

  /* For all types unrepresented in CTF, use an explicit CTF type of kind
     CTF_K_UNKNOWN.  */
  if ((type_id == CTF_NULL_TYPEID) && (!unrecog_die))
    type_id = gen_ctf_unknown_type (ctfc);

  return type_id;
}

bool
ctf_do_die (dw_die_ref die)
{
  ctf_container_ref tu_ctfc = ctf_get_tu_ctfc ();

  /* Note how we tell the caller to continue traversing children DIEs
     if this DIE didn't result in CTF records being added.  */
  if (dw_get_die_tag (die) == DW_TAG_variable)
    {
      gen_ctf_variable (tu_ctfc, die);
      return false;
    }
  else if (dw_get_die_tag (die) == DW_TAG_subprogram)
    {
      gen_ctf_function (tu_ctfc, die);
      return false;
    }
  else
    return gen_ctf_type (tu_ctfc, die) == CTF_NULL_TYPEID;
}

/* Initialize CTF subsystem for CTF debug info generation.  */

void
ctf_debug_init (void)
{
  /* First, initialize the CTF subsystem.  */
  ctf_init ();

  /* Create a couple of DIE structures that we may need.  */
  ctf_void_die = new_die_raw (DW_TAG_unspecified_type);
  add_name_attribute (ctf_void_die, "void");
  ctf_array_index_die
    = base_type_die (integer_type_node, 0 /* reverse */);
  add_name_attribute (ctf_array_index_die, "int");
  ctf_unknown_die = new_die_raw (DW_TAG_unspecified_type);
  add_name_attribute (ctf_unknown_die, "unknown");
}

/* Preprocess the CTF debug information after initialization.  */

void
ctf_debug_init_postprocess (bool btf)
{
  /* Only BTF requires postprocessing right after init.  */
  if (btf)
    btf_init_postprocess ();
}

/* Prepare for output and write out the CTF debug information.  */

void
ctf_debug_finalize (const char *filename, bool btf)
{
  if (btf)
    {
      btf_output (filename);
      btf_finalize ();
    }

  else
    {
      /* Emit the collected CTF information.  */
      ctf_output (filename);

      /* Reset the CTF state.  */
      ctf_finalize ();
    }
}

#include "gt-dwarf2ctf.h"
