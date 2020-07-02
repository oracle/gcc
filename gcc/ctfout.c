/* Output CTF format from GCC.
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

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "target.h"
#include "rtl.h"
#include "tree.h"
#include "memmodel.h"
#include "tm_p.h"
#include "toplev.h"
#include "varasm.h"
#include "output.h"
#include "dwarf2asm.h"
#include "debug.h"
#include "ctfout.h"
#include "diagnostic-core.h"

/* A CTF container object - one per translation unit.  */

static GTY (()) ctf_container_ref tu_ctfc;

static int ctf_label_num;

/* Pointers to various CTF sections.  */

static GTY (()) section * ctf_info_section;

/* Section names used to hold CTF debugging information.  */

/* CTF debug info section.  */

#ifndef CTF_INFO_SECTION_NAME
#define CTF_INFO_SECTION_NAME  ".ctf"
#endif

/* Section flags for the CTF debug info section.  */

#define CTF_INFO_SECTION_FLAGS (SECTION_DEBUG)

/* Maximum size (in bytes) of an artificially generated CTF label.  */

#define MAX_CTF_LABEL_BYTES 40

static char ctf_info_section_label[MAX_CTF_LABEL_BYTES];

#ifndef CTF_INFO_SECTION_LABEL
#define CTF_INFO_SECTION_LABEL			"Lctf"
#endif

/* Forward declarations for some routines defined in this file.  */

/* Generate CTF type for the given type.  Types already added are skipped.  */

static ctf_id_t gen_ctf_type (ctf_container_ref, tree);

/* Generate CTF type for the given decl.  Types already added are skipped.  */

static ctf_id_t gen_ctf_type_for_decl (ctf_container_ref, tree);

/* CTF preprocess callback arguments.  */

typedef struct ctf_dtd_preprocess_arg
{
  unsigned long dtd_global_func_idx;
  ctf_container_ref dtd_arg_ctfc;
} ctf_dtd_preprocess_arg_t;

typedef struct ctf_dvd_preprocess_arg
{
  unsigned long dvd_global_obj_idx;
  ctf_container_ref dvd_arg_ctfc;
} ctf_dvd_preprocess_arg_t;

/* CTF cvr qualifier mask.  */

const int ctf_cvr_qual_mask = (TYPE_QUAL_CONST
			      | TYPE_QUAL_VOLATILE
			      | TYPE_QUAL_RESTRICT);

/* Return which member of the union is used in CTFTYPE.  Used for garbage
   collection.  */

enum ctf_dtu_d_union_enum
ctf_dtu_d_union_selector (ctf_dtdef_ref ctftype)
{
  unsigned int kind = CTF_V2_INFO_KIND (ctftype->dtd_data.ctti_info);
  switch (kind)
    {
    case CTF_K_INTEGER:
    case CTF_K_FLOAT:
      return CTF_DTU_D_ENCODING;
    case CTF_K_STRUCT:
    case CTF_K_UNION:
    case CTF_K_ENUM:
      return CTF_DTU_D_MEMBERS;
    case CTF_K_ARRAY:
      return CTF_DTU_D_ARRAY;
    case CTF_K_FUNCTION:
      return CTF_DTU_D_ARGUMENTS;
    case CTF_K_SLICE:
      return CTF_DTU_D_SLICE;
    default:
      /* The largest member as default.  */
      return CTF_DTU_D_ARRAY;
    }
}

/* Add the compilation unit (CU) name string to the the CTF string table.  The
   CU name has a prepended pwd string if it is a relative path.  Also set the
   CU name offset in the CTF container.  */

static void
ctfc_add_cuname (ctf_container_ref ctfc, const char * filename)
{
  char * cuname = NULL;

  /* (filename at this point of compilation cannot be null).  */

  if (!IS_DIR_SEPARATOR (filename[0]))
    {
      /* Filename is a relative path.  */
      const char * cu_pwd = get_src_pwd ();
      const int cu_pwd_len = strlen (cu_pwd);

      /* Add a DIR_SEPARATOR char before the filename.  */
      const int len = cu_pwd_len + 2 + strlen (filename);

      cuname = (char *) ggc_alloc_atomic (len);
      memset (cuname, 0, len);

      strcpy (cuname, cu_pwd);
      cuname[cu_pwd_len] = DIR_SEPARATOR;
      cuname[cu_pwd_len+1] = 0;
      strcat (cuname, filename);
    }
  else
    /* Filename is an absolute path.  */
    cuname = CONST_CAST (char *, ggc_strdup (filename));

  ctf_add_string (ctfc, cuname, &(ctfc->ctfc_cuname_offset));
  /* Add 1 as CTF strings in the CTF string table are null-terminated
     strings.  */
  ctfc->ctfc_strlen += strlen (cuname) + 1;

  /* Mark cuname for garbage collection.  */
  cuname = NULL;
}

/* Returns a hash code for CTF type records.  */

hashval_t
ctf_dtdef_hash::hash (ctf_dtdef_ref e1)
{
  ctf_dtdef_ref e = e1;
  tree e_decl = e->dtd_decl;
  uint32_t key_flags = e->dtd_key_flags;

  hashval_t key = hash_dtd_tree_decl (e_decl, key_flags);

  return key;
}

hashval_t
hash_dtd_tree_decl (tree e_decl, uint32_t key_flags)
{
  hashval_t key;
  tree type = NULL;

  if ((TREE_CODE (e_decl) == FIELD_DECL)
      || (TREE_CODE (e_decl) == TYPE_DECL))
    type = TREE_TYPE (e_decl);
  else
    type = e_decl; /* TREE_TYPE was used as dtd_key otherwise.  */

  if (TREE_CODE (e_decl) == TYPE_DECL
      || TREE_CODE (e_decl) == FUNCTION_DECL
      /* No CTF type de-duplication for slices.  See note in
	 gen_ctf_bitfield_type_for_decl.  */
      || ((TREE_CODE (e_decl) == FIELD_DECL) && DECL_BIT_FIELD_TYPE (e_decl)))
    {
      key = (hashval_t) DECL_UID (e_decl);
    }
  else
    {
      gcc_assert (TREE_CODE_CLASS (TREE_CODE (type)) == tcc_type);
      key = (hashval_t) TYPE_UID (type);
    }

  if (key_flags)
    key = iterative_hash (&key_flags, sizeof (key_flags), key);

  return key;
}

/* Returns nonzero if entry1 and entry2 are the same CTF types.  */

bool
ctf_dtdef_hash::equal (ctf_dtdef_ref entry1, ctf_dtdef_ref entry2)
{
  bool eq = 0;
  tree e1_type, e2_type;
  int e1_cvr_quals = 0, e2_cvr_quals = 0;

  ctf_dtdef_ref e1 = entry1;
  ctf_dtdef_ref e2 = entry2;

  tree e1_decl = e1->dtd_decl;
  tree e2_decl = e2->dtd_decl;

  gcc_assert (e1_decl);
  gcc_assert (e2_decl);
  /* This pre-check is useful because dtd_decl can be either type or decl tree
     references.  */
  eq = (TREE_CODE (e1_decl) == TREE_CODE (e2_decl));
  if (eq)
    {
      if ((TREE_CODE (e1_decl) == FIELD_DECL)
	  || (TREE_CODE (e1_decl) == TYPE_DECL))
	{
	  e1_type = TREE_TYPE (e1_decl);
	  e2_type = TREE_TYPE (e2_decl);
	}
      else
	{
	  /* TREE_TYPE was used as dtd_key otherwise.  */
	  e1_type = e1_decl;
	  e2_type = e2_decl;
	}

      if (TREE_CODE (e1_decl) == TYPE_DECL
	  || TREE_CODE (e1_decl) == FUNCTION_DECL
	  /* No CTF type de-duplication for slices.  See note in
	     gen_ctf_bitfield_type_for_decl.  */
	  || ((TREE_CODE (e1_decl) == FIELD_DECL)
	      && DECL_BIT_FIELD_TYPE (e1_decl)))

	{
	  eq = (DECL_UID (e1_decl) == DECL_UID (e2_decl));
	}
      else
	{
	  gcc_assert (TREE_CODE_CLASS (TREE_CODE (e1_type)) == tcc_type);
	  gcc_assert (TREE_CODE_CLASS (TREE_CODE (e2_type)) == tcc_type);

	  eq = (TYPE_UID (e1_type) == TYPE_UID (e2_type));

	  /* Always compare cvr_quals when available.  */
	  e1_cvr_quals = (TYPE_QUALS_NO_ADDR_SPACE_NO_ATOMIC (e1_type)
			  & ctf_cvr_qual_mask);
	  e2_cvr_quals = (TYPE_QUALS_NO_ADDR_SPACE_NO_ATOMIC (e2_type)
			  & ctf_cvr_qual_mask);

	  if (eq && e1_cvr_quals)
	    {
	      e2_cvr_quals = (TYPE_QUALS_NO_ADDR_SPACE_NO_ATOMIC (e2_type)
			      & ctf_cvr_qual_mask);
	      eq = (e1_cvr_quals == e2_cvr_quals);
	    }
	}

      if (eq)
	{
	  /* dtd_key_flags are set only for CTF type records which have no
	     direct corresponding tree type or decl.  They will be 0
	     otherwise.  */
	  eq = (e1->dtd_key_flags == e2->dtd_key_flags);
	}
    }

  return eq;
}

static inline int
is_ctf_base_type (tree type)
{
  switch (TREE_CODE (type))
    {
    case INTEGER_TYPE:
    case REAL_TYPE:
    case FIXED_POINT_TYPE:
    case COMPLEX_TYPE:
    case BOOLEAN_TYPE:
    case VOID_TYPE:
      return 1;

    case ARRAY_TYPE:
    case RECORD_TYPE:
    case UNION_TYPE:
    case QUAL_UNION_TYPE:
    case ENUMERAL_TYPE:
    case FUNCTION_TYPE:
    case METHOD_TYPE:
    case POINTER_TYPE:
    case REFERENCE_TYPE:
    case NULLPTR_TYPE:
    case OFFSET_TYPE:
    case LANG_TYPE:
    case VECTOR_TYPE:
      return 0;

    default:
      gcc_unreachable ();
    }

  return 0;
}

static inline int
get_cvr_quals_for_type (tree type)
{
  int cvr_quals = 0;

  if (TREE_CODE_CLASS (TREE_CODE (type)) == tcc_type)
    cvr_quals = TYPE_QUALS_NO_ADDR_SPACE_NO_ATOMIC (type);

  return cvr_quals;
}

static const char *
get_type_name_string (tree type)
{
  gcc_assert (TREE_CODE_CLASS (TREE_CODE (type)) == tcc_type);

  tree type_name = TYPE_IDENTIFIER (type);
  const char * name_string = type_name ? IDENTIFIER_POINTER (type_name) : NULL;

  return name_string;
}

static const char *
get_decl_name_string (tree decl)
{
  gcc_assert (TREE_CODE_CLASS (TREE_CODE (decl)) == tcc_declaration);

  tree decl_name = DECL_NAME (decl);
  const char * name_string = decl_name ? IDENTIFIER_POINTER (decl_name) : NULL;

  return name_string;
}

/* Check if CTF for TYPE has already been generated.  Mainstay for
   de-duplication.  If CTF type already exists, returns TRUE and updates
   the TYPE_ID for the caller.  */

static bool
ctf_type_exists (ctf_container_ref ctfc, tree type,
		 ctf_id_t * type_id)
{
  bool exists = false;
  uint32_t key_flags = 0;

  /* In the same compilation unit, use key_flags to differentiate a forward
     type from the struct/union/enum definition which may follow later.  Both
     must be CTF encoded for completeness.  */
  if (RECORD_OR_UNION_TYPE_P (type) || TREE_CODE (type) == ENUMERAL_TYPE)
    key_flags = (TYPE_SIZE (type) == NULL);

  ctf_dtdef_ref ctf_type_seen
    = ctf_dtd_lookup_with_flags (ctfc, type, key_flags);

  if (ctf_type_seen)
    {
      exists = true;
      /* CTF type for this type exists.  */
      *type_id = ctf_type_seen->dtd_type;
    }

  return exists;
}

/* CTF container setup and teardown routines.  */

/* Initialize the CTF string table.
   The first entry in the CTF string table (empty string) is added.  */

static void
init_ctf_string_table (ctf_container_ref ctfc)
{
  ctfc->ctfc_strtable.ctstab_head = NULL;
  ctfc->ctfc_strtable.ctstab_tail = NULL;
  ctfc->ctfc_strtable.ctstab_num = 0;
  ctfc->ctfc_strtable.ctstab_len = 0;

  /* The first entry in the CTF string table is an empty string.  E.g., CTF
     type records with no name (like CTF_K_CONST, CTF_K_VOLATILE etc) point to
     this string.  */
  uint32_t estr_offset = 0;
  ctfc->ctfc_strtable.ctstab_estr = ctf_add_string (ctfc, "", &estr_offset);
  ctfc->ctfc_strlen++;
}

/* Allocate a new CTF container with the desired flags.  */

static inline ctf_container_ref
new_ctf_container (unsigned char ctp_flags)
{
  tu_ctfc = ggc_cleared_alloc<ctf_container_t> ();

  tu_ctfc->ctfc_magic = CTF_MAGIC;
  tu_ctfc->ctfc_version = CTF_VERSION;
  tu_ctfc->ctfc_flags = ctp_flags;
  tu_ctfc->ctfc_nextid = CTF_INIT_TYPEID;

  tu_ctfc->ctfc_types
    = hash_map<ctf_dtdef_hash, ctf_dtdef_ref>::create_ggc (100);

  tu_ctfc->ctfc_vars
    = hash_map<tree_decl_hash, ctf_dvdef_ref>::create_ggc (100);

  init_ctf_string_table (tu_ctfc);

  return tu_ctfc;
}

/* Initialize a CTF container per translation unit.  */

static void
init_ctf_containers (void)
{
  tu_ctfc = new_ctf_container (0);
}

/* Delete the CTF container's resources.  */

static void
delete_ctf_container (ctf_container_ref ctfc)
{
  /* FIXME - CTF container can be cleaned up now.
     Will the ggc machinery take care of cleaning up the container structure
     including the hash_map members etc. ?  */
  if (ctfc)
    {
      if (ctfc->ctfc_vars_list)
	{
	  ggc_free (ctfc->ctfc_vars_list);
	  ctfc->ctfc_vars_list = NULL;
	}
      if (ctfc->ctfc_types_list)
	{
	  ggc_free (ctfc->ctfc_types_list);
	  ctfc->ctfc_types_list = NULL;
	}
      if (ctfc->ctfc_gfuncs_list)
	{
	  ggc_free (ctfc->ctfc_gfuncs_list);
	  ctfc->ctfc_gfuncs_list = NULL;
	}
      if (ctfc->ctfc_gobjts_list)
	{
	  ggc_free (ctfc->ctfc_gobjts_list);
	  ctfc->ctfc_gobjts_list = NULL;
	}

      ctfc= NULL;
    }
}

/* Initialize the various sections and labels for CTF output.  */

void
init_ctf_sections (void)
{
  /* Note : Even in case of LTO, the compiler continues to generate a single
     CTF section for each compilation unit "early".  Unlike other debug
     sections, CTF sections are non-LTO sections, and do not take the
     .gnu.debuglto_ prefix.  The linker will de-duplicate the types in the CTF
     sections, in case of LTO or  otherwise.  */
  ctf_info_section = get_section (CTF_INFO_SECTION_NAME, CTF_INFO_SECTION_FLAGS,
				  NULL);

  ASM_GENERATE_INTERNAL_LABEL (ctf_info_section_label,
			       CTF_INFO_SECTION_LABEL, ctf_label_num++);
}

/* Leaf routines for CTF type generation.
   Called via the gen_ctf_type (), these APIs update the CTF container with new
   CTF records.  In most cases, CTF type de-duplication must be done by the
   caller API.
   (See "Parent routines for CTF generation below).  */

/* Generate CTF for base type (integer, boolean, real, fixed point and complex).
   Important: the caller of this API must make sure that duplicate types are
   not added.  */

static ctf_id_t
gen_ctf_base_type (ctf_container_ref ctfc, tree type)
{
  ctf_id_t type_id = CTF_NULL_TYPEID;

  ctf_encoding_t ctf_encoding = {0, 0, 0};
  HOST_WIDE_INT size = int_size_in_bytes (type);

  uint32_t encoding = 0;
  bool ctf_not_supported = false;

  const char * name_string = get_type_name_string (type);
  /* Base TYPE node must have had a TYPE_IDENTIFIER node, else retrieval of
     name string of the base type will need to be adjusted.  */
  /* This assert here fails for "complex char a".  CTF skips these types.  But
     need to debug why the TYPE_NAME is null.  FIXME - moved the asserts into
     each respective block below meanwhile.  */
  // gcc_assert (name_string);

  /* Add the type of variable.  */
  switch (TREE_CODE (type))
    {
    case INTEGER_TYPE:
      {
	/* Note - CTF_INT_VARARGS is unused in CTF.  */

	/* Update size and encoding.  */
	if (TYPE_STRING_FLAG (type))
	  {
	    if (TYPE_UNSIGNED (type))
	      encoding = CTF_INT_CHAR;
	    else
	      encoding = CTF_INT_CHAR | CTF_INT_SIGNED;
	  }
	else if (!TYPE_UNSIGNED (type))
	  encoding = CTF_INT_SIGNED;

	ctf_encoding.cte_format = encoding;
	ctf_encoding.cte_bits = size * BITS_PER_UNIT;

	gcc_assert (name_string);
	type_id = ctf_add_integer (ctfc, CTF_ADD_ROOT, name_string,
				   &ctf_encoding, type);

	break;
      }

  case BOOLEAN_TYPE:
    encoding = CTF_INT_BOOL;

    ctf_encoding.cte_format = encoding;
    ctf_encoding.cte_bits = size * BITS_PER_UNIT;

    gcc_assert (name_string);
    type_id = ctf_add_integer (ctfc, CTF_ADD_ROOT, name_string,
			       &ctf_encoding, type);
    break;

  case REAL_TYPE:
    if (FLOAT_MODE_P (TYPE_MODE (type)))
      {
	tree float_type = TYPE_MAIN_VARIANT (type);

	if (float_type == float_type_node)
	  encoding = CTF_FP_SINGLE;
	else if (float_type == double_type_node)
	  encoding = CTF_FP_DOUBLE;
	else if (float_type == long_double_type_node)
	  encoding = CTF_FP_LDOUBLE;
	else
	  {
	    /* CTF does not have representation for IEEE interchange and
	       extended types.  Skip them.
	       FIXME - Note this TBD_CTF_REPRESENTATION_LIMIT.  */
	    for (int i = 0; i < NUM_FLOATN_NX_TYPES; i++)
	      {
		tree floatn_nx_type = FLOATN_NX_TYPE_NODE (i);
		if (float_type == floatn_nx_type)
		  {
		    ctf_not_supported = true;
		    break;
		  }
	      }
	    /* CTF does not have representation for Decimal float type.
	       FIXME - Note this TBD_CTF_REPRESENTATION_LIMIT.  */
	    if (DECIMAL_FLOAT_TYPE_P (float_type))
	      ctf_not_supported = true;
	    if (ctf_not_supported)
	      break;
	  }

	/* Encoding must be appropriately initialized by now.  */
	gcc_assert (encoding && encoding <= CTF_FP_MAX);

	ctf_encoding.cte_format = encoding;
	ctf_encoding.cte_bits = size * BITS_PER_UNIT;

	gcc_assert (name_string);
	type_id = ctf_add_float (ctfc, CTF_ADD_ROOT, name_string,
			       &ctf_encoding, type);
      }

    /* CTF does not have representation for non IEEE float encoding.  Skip
       this type.  FIXME - Note this TBD_CTF_REPRESENTATION_LIMIT.  */
    break;

  case FIXED_POINT_TYPE:
    /* CTF does not have representation for fixed point type.  Skip this type.
       FIXME - Note this TBD_CTF_REPRESENTATION_LIMIT.  */
    break;

  case COMPLEX_TYPE:
    encoding = 0;
    if (TREE_CODE (TREE_TYPE (type)) == REAL_TYPE)
      {
	tree component_type = TYPE_MAIN_VARIANT (TREE_TYPE (type));
	size = int_size_in_bytes (component_type);

	if (component_type == float_type_node)
	  encoding = CTF_FP_CPLX;
	else if (component_type == double_type_node)
	  encoding = CTF_FP_DCPLX;
	else if (component_type == long_double_type_node)
	  encoding = CTF_FP_LDCPLX;
	else
	  {
	    /* CTF does not have representation for IEEE interchange and
	       extended types.  Skip them.
	       FIXME - Note this TBD_CTF_REPRESENTATION_LIMIT.  */
	    for (int i = 0; i < NUM_FLOATN_NX_TYPES; i++)
	      {
		tree floatn_nx_type = FLOATN_NX_TYPE_NODE (i);
		if (component_type == floatn_nx_type)
		  {
		    ctf_not_supported = true;
		    break;
		  }
	      }
	    /* CTF does not have representation for Decimal float type.
	       FIXME - Note this TBD_CTF_REPRESENTATION_LIMIT.  */
	    if (DECIMAL_FLOAT_TYPE_P (component_type))
	      ctf_not_supported = true;
	    if (ctf_not_supported)
	      break;
	  }

	/* Encoding must be appropriately initialized by now.  */
	gcc_assert (encoding && encoding != CTF_FP_MAX);

	ctf_encoding.cte_format = encoding;
	ctf_encoding.cte_bits = size * BITS_PER_UNIT;

	gcc_assert (name_string);
	type_id = ctf_add_float (ctfc, CTF_ADD_ROOT, name_string,
				 &ctf_encoding, type);
      }
    /* CTF does not have representation for complex integer type.  Skip this
       type.  FIXME - Note this TBD_CTF_REPRESENTATION_LIMIT.  */
    break;

  case VOID_TYPE:
    encoding = CTF_INT_SIGNED;
    ctf_encoding.cte_format = encoding;
    ctf_encoding.cte_bits = 0;

    gcc_assert (name_string);
    type_id = ctf_add_integer (ctfc, CTF_ADD_ROOT, name_string,
			       &ctf_encoding, type);

    break;

  default:
    /* No other TREE_CODEs are expected as CTF base types.  */
    gcc_unreachable () ;
  }

  return type_id;
}

/* Generate CTF typedef record for a given declaration.  */
static ctf_id_t
gen_ctf_typedef_type (ctf_container_ref ctfc, tree type, ctf_id_t type_id)
{
  ctf_id_t typedef_type_id = CTF_NULL_TYPEID;

  tree decl = TYPE_NAME (type);
  const char * decl_name_string = get_type_name_string (type);

  gcc_assert (type_id != CTF_NULL_TYPEID);
  gcc_assert (TREE_CODE (decl) == TYPE_DECL);
  gcc_assert (typedef_variant_p (type));

  /* Add typedef.  dtd_decl points to the typedef tree node.  */
  typedef_type_id = ctf_add_typedef (ctfc, CTF_ADD_ROOT, decl_name_string,
				     type_id, decl);
  type_id = typedef_type_id;

  return type_id;
}

static ctf_id_t
gen_ctf_pointer_type (ctf_container_ref ctfc, tree ptr_type)
{
  ctf_id_t type_id = CTF_NULL_TYPEID;
  ctf_id_t pointer_to_type_id = CTF_NULL_TYPEID;

  tree pointertotype = TREE_TYPE (ptr_type);

  type_id = gen_ctf_type (ctfc, pointertotype);

  /* Type de-duplication.
     Consult the ctfc_types hash again before adding the CTF pointer type
     because there can be cases where a pointer type may have been added by
     the gen_ctf_type call above.  For example, a struct have a member of type
     pointer to the struct, e.g.,
     struct link { struct link * next; } * slink;  */
  if (ctf_type_exists (ctfc, ptr_type, &pointer_to_type_id))
    return pointer_to_type_id;

  pointer_to_type_id = ctf_add_pointer (ctfc, CTF_ADD_ROOT, type_id,
					  ptr_type);

  return pointer_to_type_id;
}

static ctf_id_t
gen_ctf_array_type (ctf_container_ref ctfc, tree array_type)
{
  ctf_id_t type_id = CTF_NULL_TYPEID;
  tree lower, upper;
  ctf_arinfo_t arinfo;
  /* Init max_index to -1.  */
  HOST_WIDE_INT min_index = 0, max_index = -1;
  uint32_t num_elements = 0;
  ctf_id_t ctf_contents_type_id = CTF_NULL_TYPEID;
  ctf_id_t ctf_index_type_id = CTF_NULL_TYPEID;

  tree type_of_array_element = TREE_TYPE (array_type);
  tree type_of_index = TYPE_DOMAIN (array_type);

  /* type_of_index may be NULL in some cases, e.g., when we parse
     extern const char _var_example[];
     In this case of unsized uninitialized array declaration, CTF encodes an
     explicit zero for the number of elements.  This is quite distinct from
     DWARF which encodes no bound information in such a case.
     TBD_CTF_FORMAT_OPEN_ISSUES (1) - see testcase ctf-array-2.c.  */
  if (type_of_index)
    {
      lower = TYPE_MIN_VALUE (type_of_index);
      upper = TYPE_MAX_VALUE (type_of_index);
      min_index = tree_to_shwi (lower);

      /* TYPE_MAX_VALUE of index may be null for variable-length arrays.

	 There may be other cases where the upper bound of an array is not a
	 constant.  For example - int foo (int a, int b[a][a]).  These are but
	 another flavor of vla's.  Use a max_index of 0 in these cases.
	 TBD_CTF_FORMAT_OPEN_ISSUES (1) - see testcase ctf-array-3.c.  */
      if (upper && tree_fits_shwi_p (upper))
	max_index = tree_to_shwi (upper);

      if (max_index > 0) gcc_assert (max_index >= min_index);
      /* If max_index == min_index, both the values must be zero; num_elements
	 set to zero in that case.  */
      num_elements = (max_index >= 0 && max_index >= min_index)
		      ? max_index - min_index + 1 : 0;
      gcc_assert (num_elements <= CTF_MAX_SIZE);
    }

  arinfo.ctr_nelems = num_elements;

  /* Overwrite the type_of_index with integer_type_node.
     TYPE_DOMAIN of ARRAY_TYPE have code INTEGER_TYPE, but have no
     IDENTIFIER_NODES.  This causes issues in retrieving the name string of
     the index type (See gen_ctf_base_type) becuase the TYPE_IDENTIFIER is NULL
     in those cases.
     Use integer_type_node instead.  This also helps gen_ctf_base_type to not
     generate crooked (and duplicate) int records with wierd "integer" size for
     arrays.  */
  type_of_index = integer_type_node;

  ctf_index_type_id = gen_ctf_type (ctfc, type_of_index);
  arinfo.ctr_index = ctf_index_type_id;

  ctf_contents_type_id = gen_ctf_type (ctfc, type_of_array_element);
  arinfo.ctr_contents = ctf_contents_type_id;

  /* Type de-duplication.
     Consult the ctfc_types hash again before adding the CTF array type because
     there can be cases where an array_type type may have been added by the
     gen_ctf_type call above.  */
  if (!ctf_type_exists (ctfc, array_type, &type_id))
    type_id = ctf_add_array (ctfc, CTF_ADD_ROOT, &arinfo, array_type);

  return type_id;
}

static ctf_id_t
gen_ctf_forward_type (ctf_container_ref ctfc, tree fwd_type, uint32_t kind)
{
  ctf_id_t fwd_type_id = 0;

  const char * fwd_name = get_type_name_string (fwd_type);
  /* Type de-duplication is already done by now.  See gen_ctf_type ().
     Simple add the forward type.  */
  fwd_type_id = ctf_add_forward (ctfc, CTF_ADD_ROOT, fwd_name, kind, fwd_type);

  return fwd_type_id;
}

static void
gen_ctf_enum_const_list (ctf_container_ref ctfc, const tree enum_type,
			 const ctf_id_t enum_type_id)
{
  tree link;
  tree enum_value;
  HOST_WIDE_INT value;
  /* Append the enum values to the CTF_K_ENUM record.  */
  for (link = TYPE_VALUES (enum_type); link != NULL; link = TREE_CHAIN (link))
    {
      enum_value = TREE_VALUE (link);
      /* For now, handle enumeration constants not wider than
	 HOST_WIDE_INT.  TBD handle this.  */
      gcc_assert (int_size_in_bytes (TREE_TYPE (enum_value))*HOST_BITS_PER_CHAR
		  <= HOST_BITS_PER_WIDE_INT || tree_fits_shwi_p (enum_value));

      value = TREE_INT_CST_LOW (enum_value);
      const char * enum_valname = IDENTIFIER_POINTER (TREE_PURPOSE (link));
      gcc_assert (enum_valname);

      ctf_add_enumerator (ctfc, enum_type_id, enum_valname, value, enum_type);

      /* ctf_add_enumerator returns true if addition of the enumeration
	 constant is skipped if not representable in CTF (int32_t).
	 FIXME - Note this TBD_CTF_REPRESENTATION_LIMIT.  */
    }
}

static ctf_id_t
gen_ctf_enum_type (ctf_container_ref ctfc, tree enum_type)
{
  ctf_id_t enum_type_id = CTF_NULL_TYPEID;
  HOST_WIDE_INT size;

  gcc_assert (TREE_CODE (enum_type) == ENUMERAL_TYPE);

  if (!TYPE_SIZE (enum_type))
    {
      /* Add CTF forward type of enum kind.  */
      uint32_t kind = CTF_K_ENUM;
      enum_type_id = gen_ctf_forward_type (ctfc, enum_type, kind);
      return enum_type_id;
    }

  const char * enum_name = get_type_name_string (enum_type);
  size = int_size_in_bytes (enum_type);

  /* Add CTF enum type.  */
  enum_type_id = ctf_add_enum (ctfc, CTF_ADD_ROOT, enum_name, size, enum_type);
  /* Add CTF records for enum const values.  */
  gen_ctf_enum_const_list (ctfc, enum_type, enum_type_id);

  return enum_type_id;
}

static ctf_id_t
gen_ctf_function_type (ctf_container_ref ctfc, tree func_decl_or_type,
		       const char * func_name)
{
  ctf_id_t type_id = CTF_NULL_TYPEID, return_type_id = CTF_NULL_TYPEID;
  tree func_type;
  tree link;
  tree first_param_type;
  tree formal_type = NULL;
  tree return_type = NULL;
  tree param_type;
  uint32_t num_args = 0;
  ctf_func_arg_t * argv_ids;
  ctf_funcinfo_t func_info;

  if (TREE_CODE (func_decl_or_type) == FUNCTION_TYPE)
    func_type = func_decl_or_type;
  else
    func_type = TREE_TYPE (func_decl_or_type);

  return_type = TREE_TYPE (func_type);
  first_param_type = TYPE_ARG_TYPES (func_type);

  /* Add CTF record for function return type.  */
  return_type_id = gen_ctf_type (ctfc, return_type);
  func_info.ctc_return = return_type_id;

  /* Make our first pass over the list of formal parameter types and count
     them.  */
  for (link = first_param_type; link;)
    {
      formal_type = TREE_VALUE (link);
      if (formal_type == void_type_node)
	break;

      num_args++;

      link = TREE_CHAIN (link);
    }

  /* Check if this function type has an ellipsis.  */
  if (formal_type != void_type_node)
    {
      func_info.ctc_flags |= CTF_FUNC_VARARG;
      /* Increment the number of args.  This is the number of args we write
	 after the CTF_K_FUNCTION CTF record.  */
      num_args++;
    }

  /* The number of typed arguments should include the ellipsis.  */
  func_info.ctc_argc = num_args;

  /* Create an array of ctf_id_t to hold CTF types for args (including the
     ellipsis).  */
  argv_ids = ggc_vec_alloc<ctf_func_arg_t>(num_args);

  /* Make a pass over the list of formal parameter types and generate CTF for
     each.  */
  unsigned int i = 0;
  for (link = TYPE_ARG_TYPES (func_type);
       link && TREE_VALUE (link);
       link = TREE_CHAIN (link))
    {
      param_type = TREE_VALUE (link);

      if (param_type == void_type_node)
	break;

      argv_ids[i++].farg_type = gen_ctf_type (ctfc, param_type);
    }

  if (formal_type != void_type_node)
    {
      /* Add trailing zero to indicate varargs.  */
      argv_ids[i].farg_type = 0;
      gcc_assert (i == num_args - 1);
    }

  /* Type de-duplication.
     Consult the ctfc_types hash again before adding the CTF function type
     because there can be cases where a function type may have been added by
     the gen_ctf_type call above.  For example,
     struct callback_head {
       struct callback_head *next;
	void (*func) (struct callback_head *head);
	} __attribute__ (( aligned (sizeof (void *))));
       #define rcu_head callback_head

       typedef void (*rcu_callback_t) (struct rcu_head *head);  */
  if (ctf_type_exists (ctfc, func_decl_or_type, &type_id))
    argv_ids = NULL;
  else
    type_id = ctf_add_function (ctfc, CTF_ADD_ROOT, func_name,
				(const ctf_funcinfo_t *)&func_info, argv_ids,
				func_decl_or_type);

  return type_id;
}

/* Add CTF qualifier record.

   If there are multiple qualifiers, the recommended ordering for CTF qualifier
   records is const, volatile, restrict (from top-most to bottom-most).  */

static ctf_id_t
gen_ctf_cvrquals (ctf_container_ref ctfc, tree type, ctf_id_t type_id)
{
  tree qualified_type;
  int flags;
  uint32_t cvrint = 0;
  int quals_index = 0;

  ctf_id_t qual_type_id = type_id;
  int cvr_quals = get_cvr_quals_for_type (type);

  int quals_order[3] = { TYPE_QUAL_RESTRICT,
			 TYPE_QUAL_VOLATILE,
			 TYPE_QUAL_CONST };
  ctf_id_t (*func_ptrs[3]) (ctf_container_ref, uint32_t, ctf_id_t, tree,
			    uint32_t) = { ctf_add_restrict,
					  ctf_add_volatile,
					  ctf_add_const };
  unsigned int key_flags[3] = { CTF_K_RESTRICT, CTF_K_VOLATILE, CTF_K_CONST };
  ctf_id_t (*ctf_add_qual_func) (ctf_container_ref, uint32_t, ctf_id_t, tree,
				 uint32_t);

  qualified_type = get_qualified_type (type, cvr_quals);

  /* Type de-duplication for cvr records.
     Do not add CTF types for the same type with the matching cvr qual
     if already present.  */
  if (qualified_type)
    {
      if (ctf_type_exists (ctfc, qualified_type, &qual_type_id))
	return qual_type_id;
    }
  else
    /* If the qualified_type is NULL, use TREE_TYPE of the decl to add
       the CTF record.  CTF for unqualified type must have been added by
       now.  */
    gcc_assert (ctf_type_exists (ctfc, TYPE_MAIN_VARIANT (type),
				 &qual_type_id));

  /* CTF represents distinct type records for each qualifier (CTF_K_RESTRICT,
     CTF_K_VOLATILE, CTF_K_CONST). The records can be shared between types.
     Here we try to de-duplicate these records as well.  */
  while (cvr_quals)
    {
      flags = cvr_quals & quals_order[quals_index];
      ctf_add_qual_func = func_ptrs[quals_index];
      if (flags)
	{
	  cvrint = (cvr_quals != 0);
	  /* Reset the corresponding cvr_qual flag so that it is not processed
	     again.  */
	  cvr_quals &= ~quals_order[quals_index];

	 /* The dtd_decl of the all CTF type records should be non-null for
	    de-duplication to work.  CTF records for CVR quals of a type will
	    have the same dtd_decl in this case.  So, to prevent collisions, we
	    use dtd_decl and dtd_key_flags for creating the hashkey.  */
	  ctf_dtdef_ref qual_type_exists
	    = ctf_dtd_lookup_with_flags (ctfc, type, key_flags[quals_index]);
	  if (qual_type_exists)
	    qual_type_id = qual_type_exists->dtd_type;
	  else
	    qual_type_id = ctf_add_qual_func (ctfc, CTF_ADD_ROOT, qual_type_id,
					      type, cvrint);
	}
      quals_index++;
    }

  /* At least one CTF record must have been added or found to be duplicate
     by now.  */
  gcc_assert (qual_type_id != type_id);

  return qual_type_id;
}

static ctf_id_t
gen_ctf_sou_type (ctf_container_ref ctfc, tree sou_type)
{
  HOST_WIDE_INT sou_size;

  ctf_id_t sou_type_id = CTF_NULL_TYPEID;
  ctf_id_t field_type_id = CTF_NULL_TYPEID;

  tree field;
  HOST_WIDE_INT bit_offset = 0;

  gcc_assert (RECORD_OR_UNION_TYPE_P (sou_type));

  /* Handle anonymous record or union.  */
#if 0
  if (TYPE_NAME (sou_type) == NULL)
    {
      /* TBD - confirm this behaviour.
	 The compiler will not flatten an anonymous struct or union into its
	 parent if one exists.  Members of anonymous struct or union continue
	 to be wrappped by the respective anonymous record.  */
    }
#endif
  uint32_t kind = (TREE_CODE (sou_type) == RECORD_TYPE)
		  ? CTF_K_STRUCT : CTF_K_UNION;

  if (!TYPE_SIZE (sou_type))
    {
      /* Add CTF forward type of struct or union kind.  */
      sou_type_id = gen_ctf_forward_type (ctfc, sou_type, kind);
      return sou_type_id;
    }

    const char * sou_name = get_type_name_string (sou_type);
    sou_size = int_size_in_bytes (sou_type);

    /* Add CTF struct/union type.  */
    if ((TREE_CODE (sou_type) == RECORD_TYPE)
	|| (TREE_CODE (sou_type) == UNION_TYPE))
      sou_type_id = ctf_add_sou (ctfc, CTF_ADD_ROOT, sou_name, kind, sou_size,
				 sou_type);
    /* QUAL_UNION_TYPE not expected in C.  */
    else
      gcc_unreachable ();

    /* Add members of the struct.  */
    for (field = TYPE_FIELDS (sou_type); field != NULL_TREE;
	 field = TREE_CHAIN (field))
      {
	/* Enum members have DECL_NAME (field) as NULL.  */
	const char * field_name = get_decl_name_string (field);

	/* variable bit offsets are not handled at the moment.  */
	gcc_assert (TREE_CODE (DECL_FIELD_BIT_OFFSET (field)) == INTEGER_CST);

	bit_offset = int_bit_position (field);
	/* Add the CTF type record for the field, followed by the field
	   itself.  */
	field_type_id = gen_ctf_type_for_decl (ctfc, field);
	ctf_add_member_offset (ctfc, sou_type, field_name, field_type_id,
			       bit_offset);
      }

  return sou_type_id;
}

/* Parent routines for CTF generation.
   These routines are entry points for CTF generation.  Given a type or decl,
   these routines perform de-duplication before invoking the Leaf CTF
   generation routines for adding types.  */

/* Generate CTF variable records for a given declaration.  Performs
   de-duplication before adding variable.  */

static ctf_id_t
gen_ctf_variable (ctf_container_ref ctfc, tree decl)
{
  ctf_id_t type_id = CTF_NULL_TYPEID, var_type_id = CTF_NULL_TYPEID;

  const char* name = get_decl_name_string (decl);

  ctf_dvdef_ref var_type_seen = ctf_dvd_lookup (tu_ctfc, decl);
  /* Avoid duplicates.  A VAR_DECL is duplicate if it is the same decl node.
     See hash_dvd_tree_decl.  */
  if (!var_type_seen)
    {
      type_id = gen_ctf_type_for_decl (ctfc, decl);
      /* Now add the variable.  */
      var_type_id = ctf_add_variable (tu_ctfc, name, type_id, decl);

      /* Update global objects count.  */
      if (TREE_PUBLIC (decl))
	ctfc->ctfc_num_global_objts++;
    }
  else
    var_type_id = var_type_seen->dvd_type;

  return var_type_id;
}

/* Generate CTF function records for a given declaration.  */

static ctf_id_t
gen_ctf_function (ctf_container_ref ctfc, tree func_decl)
{
  gcc_assert (TREE_CODE (func_decl) == FUNCTION_DECL);

  ctf_id_t type_id = CTF_NULL_TYPEID;

  const char * func_name = get_decl_name_string (func_decl);

  /* Duplicate function types are expected to be seen as functions with same
     signature will show the same function type.  For each distinct function
     declaration, however, CTF function type records must be added anew.
     Duplicate function *declarations* must, however be avoided.  This is
     expected to happen if the gen_ctf_function API is called multiple times
     for the same func_decl.  */

  bool exists = ctf_type_exists (ctfc, func_decl, &type_id);
  gcc_assert (!exists);

  /* Add CTF Function type.  */
  type_id = gen_ctf_function_type (ctfc, func_decl, func_name);

  /* Update global functions count.  */
  if (TREE_PUBLIC (func_decl))
    ctfc->ctfc_num_global_funcs++;

  return type_id;
}

/* Add CTF type record(s) for the given input type.  */
static ctf_id_t
gen_ctf_type (ctf_container_ref ctfc, tree type)
{
  ctf_id_t type_id = CTF_NULL_TYPEID;

  /* This API expects to handle tcc_type nodes only.
     ctf_add_int/float etc  == type ==  INTEGER_TYPE, REAL_TYPE etc
     ctf_add_pointer	    == type ==  POINTER_TYPE
     ctf_add_array	    == type ==  ARRAY_TYPE
     ctf_add_sou	    == type ==  RECORD_TYPE, UNION_TYPE.  */
  gcc_assert (TREE_CODE_CLASS (TREE_CODE (type)) == tcc_type);

  int cvr_quals = get_cvr_quals_for_type (type);

  /* For a type of ARRAY_TYPE, type qualifiers (if any) are with
     TREE_TYPE (type).  TYPE_MAIN_VARIANT (type) however will not contain
     these quals.  Need to pass the former to gen_ctf_array_type.  */
  tree gen_type = (TREE_CODE (type) == ARRAY_TYPE)
		  ? type : TYPE_MAIN_VARIANT (type);

  /* CTF type de-duplication in the compiler.
   
     Note that the de-duplication of pointer types shall be done in
     gen_ctf_pointer_type and not here.  This is because the pointed
     type should be checked first, in case it is a sou that has been
     completed.  */
  if (POINTER_TYPE_P (type)
      || !ctf_type_exists (ctfc, gen_type, &type_id))
    {
      /* Encountering a CTF type for the first time.  Add the CTF type.  */

      if (is_ctf_base_type (gen_type))
	type_id = gen_ctf_base_type (ctfc, gen_type);

      else if (RECORD_OR_UNION_TYPE_P (gen_type))
	type_id = gen_ctf_sou_type (ctfc, gen_type);

      else if (TREE_CODE (gen_type) == ARRAY_TYPE)
	type_id = gen_ctf_array_type (tu_ctfc, gen_type);

      else if (TREE_CODE (gen_type) == ENUMERAL_TYPE)
	type_id = gen_ctf_enum_type (tu_ctfc, gen_type);

      else if (POINTER_TYPE_P (gen_type))
	type_id = gen_ctf_pointer_type (tu_ctfc, gen_type);

      else if (TREE_CODE (gen_type) == FUNCTION_TYPE)
	/* Function pointers.  */
	type_id = gen_ctf_function_type (tu_ctfc, gen_type, NULL);

      else if (TREE_CODE (gen_type) == VECTOR_TYPE)
	/* CTF does not have representation for vector types.  Skip this type.
	   FIXME- Note this TBD_CTF_REPRESENTATION_LIMIT.  */
	;

      else
	/* No other type is expected for C frontend.  */
	gcc_unreachable ();
    }

  /* Generate a CTF typedef record for all typedef of non-skipped types, if not
     generated already.  */
  if ((type_id != CTF_NULL_TYPEID)
      && typedef_variant_p (type)
      && !ctf_type_exists (ctfc, TYPE_NAME (type), &type_id))
    type_id = gen_ctf_typedef_type (ctfc, type, type_id);

  /* Add qualifiers if any.  */
  if ((type_id != CTF_NULL_TYPEID) && (cvr_quals & ctf_cvr_qual_mask))
    type_id = gen_ctf_cvrquals (ctfc, type, type_id);

  return type_id;
}

/* Generate CTF records for bitfield declarations.  */

static ctf_id_t
gen_ctf_bitfield_type_for_decl (ctf_container_ref ctfc, const tree field,
				const ctf_id_t type_id)
{
  ctf_id_t bitfield_type_id = CTF_NULL_TYPEID;

  gcc_assert (TREE_CODE (field) == FIELD_DECL);

  gcc_assert (ctfc);
  HOST_WIDE_INT size_in_bits = tree_to_shwi (DECL_SIZE (field));

  ctf_encoding_t ep = {0,0,0};
  /* Assume default encoding as unsigned.  */
  uint32_t encoding = 0;
  tree type = TREE_TYPE (field);

  if (TREE_CODE (type) == INTEGER_TYPE)
    {
      /* Handle both enum bitfields and integer bitfields.  */
      if (TYPE_STRING_FLAG (type))
	{
	  if (TYPE_UNSIGNED (type))
	    encoding = CTF_INT_CHAR;
	  else
	    encoding = CTF_INT_CHAR | CTF_INT_SIGNED;
	}
      else if (!TYPE_UNSIGNED (type))
	encoding = CTF_INT_SIGNED;
    }
  else if (TREE_CODE (type) == BOOLEAN_TYPE)
    encoding = CTF_INT_BOOL;
  else
    /* The type of a bit field can be integral or boolean.  */
    gcc_unreachable ();

  ep.cte_format = encoding;
  /* The offset of the slice is the offset into a machine word.
     TBD Handle big-endian - should the offset be byte-order dependent ?  */
  ep.cte_offset = int_bit_position (field) % BITS_PER_WORD;
  ep.cte_bits = size_in_bits;

  /* No type de-duplication for slices.
     (Probe the CTF container with field_decl)
     There is no way to de-duplicate two bitfields using just type or decl
     references as dtd_key_decl.  Two field declarations may share the
     same TREE_TYPE and DECL_BIT_FIELD_TYPE references, but may have different
     offset and num bits.  */

  bitfield_type_id = ctf_add_slice (ctfc, CTF_ADD_NONROOT, type_id, &ep,
				    field);

  return bitfield_type_id;
}

static ctf_id_t
gen_ctf_type_for_decl (ctf_container_ref ctfc, tree decl)
{
  gcc_assert (TREE_CODE_CLASS (TREE_CODE (decl)) == tcc_declaration);

  ctf_id_t type_id = CTF_NULL_TYPEID;
  tree type;
  enum tree_code code;
  tree bitfield_type = NULL;

  type = TREE_TYPE (decl);

  code = TREE_CODE (type);
  /* Use the inner type if this is an unnamed type of integer, floating point
     or fixed point type.  This can happen if mode attribute is used.  */
  if ((code == INTEGER_TYPE || code == REAL_TYPE || code == FIXED_POINT_TYPE)
      && TREE_TYPE (type) != 0 && TYPE_NAME (type) == 0)
    type = TREE_TYPE (type), code = TREE_CODE (type);

  /* In a FIELD_DECL, this indicates whether the field was a bitfield.  */
  if (TREE_CODE (decl) == FIELD_DECL)
    bitfield_type = DECL_BIT_FIELD_TYPE (decl);

  /* Create CTF for the unqualified type, if it not done already.  If it's a
     bitfield type, use that to generate the CTF type record.  */
  if (bitfield_type)
    type = bitfield_type;

  /* CTF type de-duplication in the compiler.
     Lookup if CTF for unqualified type has already been added.  CTF slices are
     not shared across declarations.  */
  if (ctf_type_exists (ctfc, type, &type_id))
    {
      if (!bitfield_type)
	return type_id;
    }
  else
    type_id = gen_ctf_type (ctfc, type);

  /* Now, create CTF slice if it is a bitfield.  */
  if (bitfield_type)
    type_id = gen_ctf_bitfield_type_for_decl (ctfc, decl, type_id);

  return type_id;
}

/* Routines for CTF pre-processing.  */

static void
ctf_preprocess_var (ctf_container_ref ctfc, ctf_dvdef_ref var)
{
  /* Add it to the list of types.  This array of types will be sorted before
     assembling into output.  */
  list_add_ctf_vars (ctfc, var);
}

/* CTF preprocess callback routine for CTF variables.  */

bool
ctf_dvd_preprocess_cb (tree const & ARG_UNUSED (key), ctf_dvdef_ref * slot,
		       void * arg)
{
  tree var_decl;

  ctf_dvd_preprocess_arg_t * dvd_arg =  (ctf_dvd_preprocess_arg_t *)arg;
  ctf_dvdef_ref var = (ctf_dvdef_ref) *slot;
  ctf_container_ref arg_ctfc = dvd_arg->dvd_arg_ctfc;

  ctf_preprocess_var (arg_ctfc, var);

  /* Keep track of global objts.  */
  var_decl = var->dvd_decl;
  if ((TREE_CODE_CLASS (TREE_CODE (var_decl)) == tcc_declaration)
      && TREE_PUBLIC (var_decl))
    {
      arg_ctfc->ctfc_gobjts_list[dvd_arg->dvd_global_obj_idx] = var;
      dvd_arg->dvd_global_obj_idx++;
    }

  return 1;
}

/* CTF preprocess callback routine for CTF types.  */

bool
ctf_dtd_preprocess_cb (ctf_dtdef_ref const & ARG_UNUSED (key),
		       ctf_dtdef_ref * slot, void * arg)
{
  uint32_t kind, vlen;
  tree func_decl;

  ctf_dtdef_ref ctftype = (ctf_dtdef_ref) *slot;
  ctf_dtd_preprocess_arg_t * dtd_arg = (ctf_dtd_preprocess_arg_t *)arg;
  ctf_container_ref arg_ctfc = dtd_arg->dtd_arg_ctfc;

  size_t index = ctftype->dtd_type;
  gcc_assert (index <= arg_ctfc->ctfc_types->elements ());

  /* CTF types need to be output in the order of their type IDs.  In other
     words, if type A is used to define type B, type ID of type A must
     appear before type ID of type B.  */
  arg_ctfc->ctfc_types_list[index] = ctftype;

  /* Keep track of the CTF type if it's a function type.  */
  kind = CTF_V2_INFO_KIND (ctftype->dtd_data.ctti_info);
  if (kind == CTF_K_FUNCTION)
    {
      func_decl = ctftype->dtd_decl;
      if ((TREE_CODE_CLASS (TREE_CODE (func_decl)) == tcc_declaration)
	  && TREE_PUBLIC (func_decl))
	{
	  arg_ctfc->ctfc_gfuncs_list[dtd_arg->dtd_global_func_idx] = ctftype;
	  dtd_arg->dtd_global_func_idx++;
	  vlen = CTF_V2_INFO_VLEN (ctftype->dtd_data.ctti_info);
	  /* Update the function info section size in bytes.  Avoid using
	     ctf_calc_num_vbytes API, the latter is only meant to convey
	     the vlen bytes after CTF types in the CTF data types section.  */
	  arg_ctfc->ctfc_num_funcinfo_bytes += (vlen + 2) * sizeof (uint32_t);
	}
    }

  /* Calculate the vlen bytes.  */
  arg_ctfc->ctfc_num_vlen_bytes += ctf_calc_num_vbytes (ctftype);

  return 1;
}

/* CTF preprocessing.
   After the CTF types for the compilation unit have been generated fully, the
   compiler writes out the asm for the CTF types.

   CTF writeout in the compiler requires two passes over the CTF types.  In the
   first pass, the CTF preprocess pass:
     1.  CTF types are sorted in the order of their type IDs.
     2.  The variable number of bytes after each CTF type record are calculated.
	 This is used to calculate the offsets in the ctf_header_t.
     3.  If the CTF type is of CTF_K_FUNCTION, the number of bytes in the
	 funcinfo sub-section are calculated.  This is used to calculate the
	 offsets in the ctf_header_t.
     4.  Keep the list of CTF variables in ASCIIbetical order of their names.

   In the second pass, the CTF writeout pass, asm tags are written out using
   the compiler's afore-generated internal pre-processed CTF types.  */

static void
ctf_preprocess (ctf_container_ref ctfc)
{
  size_t num_ctf_types = ctfc->ctfc_types->elements ();

  /* Initialize an array to keep track of the CTF variables at global
     scope.  */
  size_t num_global_objts = ctfc->ctfc_num_global_objts;
  if (num_global_objts)
    {
      ctfc->ctfc_gobjts_list = ggc_vec_alloc<ctf_dvdef_t*>(num_global_objts);
    }

  size_t num_ctf_vars = ctfc->ctfc_vars->elements ();
  if (num_ctf_vars)
    {
      ctf_dvd_preprocess_arg_t dvd_arg;
      dvd_arg.dvd_global_obj_idx = 0;
      dvd_arg.dvd_arg_ctfc = ctfc;

      /* Allocate CTF var list.  */
      ctfc->ctfc_vars_list = ggc_vec_alloc<ctf_dvdef_ref>(num_ctf_vars);
      /* Variables appear in the sort ASCIIbetical order of their names.  This
	 permits binary searching in the CTF reader.  Add the variables to a
	 list for sorting.  */
      ctfc->ctfc_vars->traverse<void *, ctf_dvd_preprocess_cb> (&dvd_arg);
      /* Sort the list.  */
      qsort (ctfc->ctfc_vars_list, num_ctf_vars, sizeof (ctf_dvdef_ref),
	     ctf_varent_compare);
    }

  /* Initialize an array to keep track of the CTF functions types for global
     functions in the CTF data section.  */
  size_t num_global_funcs = ctfc->ctfc_num_global_funcs;
  if (num_global_funcs)
    {
      ctfc->ctfc_gfuncs_list = ggc_vec_alloc<ctf_dtdef_t*>(num_global_funcs);
      gcc_assert (num_ctf_types);
    }

  if (num_ctf_types)
    {
      ctf_dtd_preprocess_arg_t dtd_arg;
      dtd_arg.dtd_global_func_idx = 0;
      dtd_arg.dtd_arg_ctfc = tu_ctfc;
      /* Allocate the CTF types list.  Add 1 because type ID 0 is never a valid
	 CTF type ID.  No CTF type record should appear at that offset, this
	 eases debugging and readability.  */
      ctfc->ctfc_types_list = ggc_vec_alloc<ctf_dtdef_ref>(num_ctf_types + 1);
      /* Pre-process CTF types.  */
      ctfc->ctfc_types->traverse<void *, ctf_dtd_preprocess_cb> (&dtd_arg);

      gcc_assert (dtd_arg.dtd_global_func_idx == num_global_funcs);
    }
}

/* CTF asm helper routines.  */

/* Asm'out the CTF preamble.  */

static void
ctf_asm_preamble (ctf_container_ref ctfc)
{
  dw2_asm_output_data (2, ctfc->ctfc_magic,
		       "CTF preamble magic number");
  dw2_asm_output_data (1, ctfc->ctfc_version, "CTF preamble version");
  dw2_asm_output_data (1, ctfc->ctfc_flags, "CTF preamble flags");
}

static void
ctf_asm_stype (ctf_dtdef_ref type)
{
  dw2_asm_output_data (4, type->dtd_data.ctti_name, "ctt_name");
  dw2_asm_output_data (4, type->dtd_data.ctti_info, "ctt_info");
  /* union.  */
  dw2_asm_output_data (4, type->dtd_data.ctti_size, "ctt_size or ctt_type");
}

static void
ctf_asm_type (ctf_dtdef_ref type)
{
  dw2_asm_output_data (4, type->dtd_data.ctti_name, "ctt_name");
  dw2_asm_output_data (4, type->dtd_data.ctti_info, "ctt_info");
  /* union.  */
  dw2_asm_output_data (4, type->dtd_data.ctti_size, "ctt_size");
  dw2_asm_output_data (4, type->dtd_data.ctti_lsizehi, "ctt_lsizehi");
  dw2_asm_output_data (4, type->dtd_data.ctti_lsizelo, "ctt_lsizelo");
}

static void
ctf_asm_slice (ctf_dtdef_ref type)
{
  dw2_asm_output_data (4, type->dtd_u.dtu_slice.cts_type, "cts_type");
  dw2_asm_output_data (2, type->dtd_u.dtu_slice.cts_offset, "cts_offset");
  dw2_asm_output_data (2, type->dtd_u.dtu_slice.cts_bits, "cts_bits");
}

static void
ctf_asm_array (ctf_dtdef_ref dtd)
{
  dw2_asm_output_data (4, dtd->dtd_u.dtu_arr.ctr_contents, "cta_contents");
  dw2_asm_output_data (4, dtd->dtd_u.dtu_arr.ctr_index, "cta_index");
  dw2_asm_output_data (4, dtd->dtd_u.dtu_arr.ctr_nelems, "cta_nelems");
}

static void
ctf_asm_varent (ctf_dvdef_ref var)
{
  /* Output the reference to the name in the string table.  */
  dw2_asm_output_data (4, var->dvd_name_offset, "ctv_name");
  /* Output the type index.  */
  dw2_asm_output_data (4, var->dvd_type, "ctv_typeidx");
}

static void
ctf_asm_sou_lmember (ctf_dmdef_t * dmd)
{
  dw2_asm_output_data (4, dmd->dmd_name_offset, "ctlm_name");
  dw2_asm_output_data (4, CTF_OFFSET_TO_LMEMHI (dmd->dmd_offset),
		       "ctlm_offsethi");
  dw2_asm_output_data (4, dmd->dmd_type, "ctlm_type");
  dw2_asm_output_data (4, CTF_OFFSET_TO_LMEMLO (dmd->dmd_offset),
		       "ctlm_offsetlo");
}

static void
ctf_asm_sou_member (ctf_dmdef_t * dmd)
{
  dw2_asm_output_data (4, dmd->dmd_name_offset, "ctm_name");
  dw2_asm_output_data (4, dmd->dmd_offset, "ctm_offset");
  dw2_asm_output_data (4, dmd->dmd_type, "ctm_type");
}

static void
ctf_asm_enum_const (ctf_dmdef_t * dmd)
{
  dw2_asm_output_data (4, dmd->dmd_name_offset, "cte_name");
  dw2_asm_output_data (4, dmd->dmd_value, "cte_value");
}

/* CTF writeout to asm file.  */

static void
output_ctf_header (ctf_container_ref ctfc)
{
  switch_to_section (ctf_info_section);
  ASM_OUTPUT_LABEL (asm_out_file, ctf_info_section_label);

  ctf_asm_preamble (ctfc);

  /* For a single compilation unit, the parent container's name and label are
     NULL.  */
  dw2_asm_output_data (4, 0, "cth_parlabel");
  dw2_asm_output_data (4, 0, "cth_parname");
  dw2_asm_output_data (4, ctfc->ctfc_cuname_offset, "cth_cuname");

  int typeslen = 0;
  /* Initialize the offsets.  The offsets are from after the CTF header.  */
  uint32_t lbloff = 0;
  uint32_t objtoff = 0;
  uint32_t funcoff = 0;
  uint32_t objtidxoff = 0;
  uint32_t funcidxoff = 0;
  uint32_t varoff = 0;
  uint32_t typeoff = 0;
  uint32_t stroff = 0;

  if (! is_empty_container (ctfc))
    {
      gcc_assert (get_num_ctf_types (ctfc)
		  == (ctfc->ctfc_num_types + ctfc->ctfc_num_stypes));

      funcoff = objtoff + ctfc->ctfc_num_global_objts * sizeof (uint32_t);
      /* Object index appears after function info.  */
      objtidxoff = funcoff + get_ctfc_num_funcinfo_bytes (ctfc);
      /* Funxtion index goes next.  */
      funcidxoff = objtidxoff + ctfc->ctfc_num_global_objts * sizeof (uint32_t);
      /* Vars appear after function index.  */
      varoff = funcidxoff + ctfc->ctfc_num_global_funcs * sizeof (uint32_t);
      /* CTF types appear after vars.  */
      typeoff = varoff + get_num_ctf_vars (ctfc) * sizeof (ctf_varent_t);
      /* The total number of bytes for CTF types is the sum of the number of
	 times struct ctf_type_t, struct ctf_stype_t are written, plus the
	 amount of variable length data after each one of these.  */
      typeslen = ctfc->ctfc_num_types * sizeof (ctf_type_t)
		+ ctfc->ctfc_num_stypes * (sizeof (ctf_stype_t))
		+ get_ctfc_num_vlen_bytes (ctfc);

      /* Strings appear after types.  */
      stroff = typeoff + typeslen;
    }

    /* Offset of label section.  */
    dw2_asm_output_data (4, lbloff, "cth_lbloff");
    /* Offset of object section.  */
    dw2_asm_output_data (4, objtoff, "cth_objtoff");
    /* Offset of function section.  */
    dw2_asm_output_data (4, funcoff, "cth_funcoff");
    /* Offset of object index section.  */
    dw2_asm_output_data (4, objtidxoff, "cth_objtidxoff");
    /* Offset of function index section.  */
    dw2_asm_output_data (4, funcidxoff, "cth_funcidxoff");

    /* Offset of variable section.  */
    dw2_asm_output_data (4, varoff, "cth_varoff");
    /* Offset of type section.  */
    dw2_asm_output_data (4, typeoff, "cth_typeoff");
    /* Offset of string section.  */
    dw2_asm_output_data (4, stroff, "cth_stroff");
    /* Length of string section in bytes.  */
    dw2_asm_output_data (4, ctfc->ctfc_strlen, "cth_strlen");
}

static void
output_ctf_obj_info (ctf_container_ref ctfc)
{
  unsigned long i;
  ctf_dvdef_ref var;

  if (!ctfc->ctfc_num_global_objts) return;

  /* Compiler spits out the objts (at global scope) in the CTF obj info section.
     In no specific order.  In an object file, the CTF object index section is
     used to associate the objts to their corresponding names.  */
  for (i = 0; i < ctfc->ctfc_num_global_objts; i++)
    {
      var = ctfc->ctfc_gobjts_list[i];

      /* CTF type ID corresponding to the type of the variable.  */
      dw2_asm_output_data (4, var->dvd_type, "objtinfo_var_type");
    }

}

static void
output_ctf_func_info (ctf_container_ref ctfc)
{
  unsigned long i, j;
  ctf_dtdef_ref ctftype;
  uint32_t vlen;

  if (!ctfc->ctfc_num_global_funcs) return;

  /* Compiler spits out the function type, return type, and args of each global
     function in the CTF funcinfo section.  In no specific order.
     In an object file, the CTF function index section is used to associate
     functions to their corresponding names.  */
  for (i = 0; i < ctfc->ctfc_num_global_funcs; i++)
    {
      ctftype = ctfc->ctfc_gfuncs_list[i];
      vlen = CTF_V2_INFO_VLEN (ctftype->dtd_data.ctti_info);

      /* function type.  */
      dw2_asm_output_data (4, ctftype->dtd_type, "funcinfo_func_type");

      /* return type.  */
      dw2_asm_output_data (4, ctftype->dtd_data.ctti_type,
			   "funcinfo_func_return_type");

      /* function args types.  */
      for (j = 0; j < vlen; j++)
	dw2_asm_output_data (4, ctftype->dtd_u.dtu_argv[j].farg_type,
			     "funcinfo_func_args");
    }
}

static void
output_ctf_objtidx (ctf_container_ref ctfc)
{
  unsigned long i;
  ctf_dvdef_ref var;

  if (!ctfc->ctfc_num_global_objts) return;

  for (i = 0; i < ctfc->ctfc_num_global_objts; i++)
    {
      var = ctfc->ctfc_gobjts_list[i];
      /* Offset to the name in CTF string table.  */
      dw2_asm_output_data (4, var->dvd_name_offset, "objtinfo_name");
    }
}

static void
output_ctf_funcidx (ctf_container_ref ctfc)
{
  unsigned long i;
  ctf_dtdef_ref ctftype;

  if (!ctfc->ctfc_num_global_funcs) return;

  for (i = 0; i < ctfc->ctfc_num_global_funcs; i++)
    {
      ctftype = ctfc->ctfc_gfuncs_list[i];
      /* Offset to the name in CTF string table.  */
      dw2_asm_output_data (4, ctftype->dtd_data.ctti_name, "funcinfo_name");
    }
}

/* Output the CTF variables.  Variables appear in the sort ASCIIbetical order
   of their names.  This permits binary searching in the CTF reader.  */

static void
output_ctf_vars (ctf_container_ref ctfc)
{
  size_t i;
  size_t num_ctf_vars = ctfc->ctfc_vars->elements ();
  if (num_ctf_vars)
    {
      /* Iterate over the list of sorted vars and output the asm.  */
      for (i = 0; i < num_ctf_vars; i++)
	ctf_asm_varent (ctfc->ctfc_vars_list[i]);
    }
}

/* Output the CTF string records.  */

static void
output_ctf_strs (ctf_container_ref ctfc)
{
  ctf_string_t * ctf_string = ctfc->ctfc_strtable.ctstab_head;

  while (ctf_string)
    {
      dw2_asm_output_nstring (ctf_string->cts_str, -1, "ctf_string");
      ctf_string = ctf_string->cts_next;
    }
}

static void
output_asm_ctf_sou_fields (ctf_container_ref ARG_UNUSED (ctfc),
			   ctf_dtdef_ref dtd)
{
  ctf_dmdef_t * dmd;

  /* Function pointer to dump struct/union members.  */
  void (*ctf_asm_sou_field_func) (ctf_dmdef_t *);

  uint32_t size = dtd->dtd_data.ctti_size;

  /* The variable length data struct/union CTF types is an array of
     ctf_member or ctf_lmember, depending on size of the member.  */
  if (size >= CTF_LSTRUCT_THRESH)
    ctf_asm_sou_field_func = ctf_asm_sou_lmember;
  else
    ctf_asm_sou_field_func = ctf_asm_sou_member;

  for (dmd = dtd->dtd_u.dtu_members;
       dmd != NULL; dmd = (ctf_dmdef_t *) ctf_dmd_list_next (dmd))
    ctf_asm_sou_field_func (dmd);
}

static void
output_asm_ctf_enum_list (ctf_container_ref ARG_UNUSED (ctfc),
			  ctf_dtdef_ref dtd)
{
  ctf_dmdef_t * dmd;

  for (dmd = dtd->dtd_u.dtu_members;
       dmd != NULL; dmd = (ctf_dmdef_t *) ctf_dmd_list_next (dmd))
    ctf_asm_enum_const (dmd);
}

static void
output_asm_ctf_vlen_bytes (ctf_container_ref ctfc, ctf_dtdef_ref ctftype)
{
  uint32_t encoding;
  uint32_t kind = CTF_V2_INFO_KIND (ctftype->dtd_data.ctti_info);
  uint32_t vlen = CTF_V2_INFO_VLEN (ctftype->dtd_data.ctti_info);
  uint32_t i;

  switch (kind)
    {
      case CTF_K_INTEGER:
      case CTF_K_FLOAT:
	if (kind == CTF_K_INTEGER)
	  {
	    encoding = CTF_INT_DATA (ctftype->dtd_u.dtu_enc.cte_format,
				     ctftype->dtd_u.dtu_enc.cte_offset,
				     ctftype->dtd_u.dtu_enc.cte_bits);
	  }
	else
	  {
	    encoding = CTF_FP_DATA (ctftype->dtd_u.dtu_enc.cte_format,
				    ctftype->dtd_u.dtu_enc.cte_offset,
				    ctftype->dtd_u.dtu_enc.cte_bits);
	  }
	dw2_asm_output_data (4, encoding, "ctf_encoding_data");
	break;
      case CTF_K_FUNCTION:
	  {
	    for (i = 0; i < vlen; i++)
	      dw2_asm_output_data (4, ctftype->dtd_u.dtu_argv[i].farg_type,
				   "dtu_argv");
	    /* FIXME - CTF_PADDING_FOR_ALIGNMENT.
	       libctf expects this padding for alignment reasons.  Expected to
	       be redundant in CTF_VERSION_4.  */
	    if (vlen & 1)
	      dw2_asm_output_data (4, 0, "dtu_argv_padding");

	    break;
	  }
      case CTF_K_ARRAY:
	ctf_asm_array (ctftype);
	break;
      case CTF_K_SLICE:
	ctf_asm_slice (ctftype);
	break;

      case CTF_K_STRUCT:
      case CTF_K_UNION:
	output_asm_ctf_sou_fields (ctfc, ctftype);
	break;
      case CTF_K_ENUM:
	output_asm_ctf_enum_list (ctfc, ctftype);
	break;

      default:
	/* CTF types of kind CTF_K_VOLATILE, CTF_K_CONST, CTF_K_RESTRICT,
	   etc have no vlen data to write.  */
	break;
    }
}

static void
output_asm_ctf_type (ctf_container_ref ctfc, ctf_dtdef_ref type)
{
  if (type->dtd_data.ctti_size <= CTF_MAX_SIZE)
    ctf_asm_stype (type);
  else
    ctf_asm_type (type);
  /* Now comes the variable-length portion for defining types completely.
     E.g., encoding follows CTF_INT_DATA, CTF_FP_DATA types,
     struct ctf_array_t follows CTF_K_ARRAY types, or a bunch of
     struct ctf_member / ctf_lmember ctf_enum sit in there for CTF_K_STRUCT or
     CTF_K_UNION.  */
  output_asm_ctf_vlen_bytes (ctfc, type);
}

/* Output the CTF type records.  */

static void
output_ctf_types (ctf_container_ref ctfc)
{
  size_t i;
  size_t num_ctf_types = ctfc->ctfc_types->elements ();
  if (num_ctf_types)
    {
      /* Type ID = 0 is used as sentinel value; not a valid type.  */
      for (i = 1; i <= num_ctf_types; i++)
	output_asm_ctf_type (ctfc, ctfc->ctfc_types_list[i]);
    }
}

static void
ctf_decl (tree decl)
{
  switch (TREE_CODE (decl))
    {
    case ERROR_MARK:
      return;

    case FUNCTION_DECL:
      /* Exclude generating CTF for functions not at file scope.  */
      if (DECL_CONTEXT (decl) == NULL_TREE
	  || TREE_CODE (DECL_CONTEXT (decl)) == TRANSLATION_UNIT_DECL)
	gen_ctf_function (tu_ctfc, decl);

      break;

    case VAR_DECL:
      gen_ctf_variable (tu_ctfc, decl);
      break;

    case TYPE_DECL:
      /* Exclude generating CTF for types not at file scope.  */
      if (DECL_CONTEXT (decl) == NULL_TREE
	  || TREE_CODE (DECL_CONTEXT (decl)) == TRANSLATION_UNIT_DECL)
	gen_ctf_type (tu_ctfc, TREE_TYPE (decl));

      break;

    default:
      if (TREE_CODE (decl) != PARM_DECL)
	/* FIXME - Its not clear why PARM_DECL arise here.  */
	/* No other TREE_CODE is expected at this time.  */
	gcc_unreachable ();
    }
}

/* CTF routines interfacing to the compiler.  */

void
ctf_debug_init (void)
{
  init_ctf_containers ();
}

void
ctf_early_finish (const char * filename)
{
  if (ctf_debug_info_level == CTFINFO_LEVEL_NONE)
    return;

  init_ctf_sections ();

  ctfc_add_cuname (tu_ctfc, filename);

  /* Pre-process CTF before generating assembly.  */
  ctf_preprocess (tu_ctfc);

  output_ctf_header (tu_ctfc);
  output_ctf_obj_info (tu_ctfc);
  output_ctf_func_info (tu_ctfc);
  output_ctf_objtidx (tu_ctfc);
  output_ctf_funcidx (tu_ctfc);
  output_ctf_vars (tu_ctfc);
  output_ctf_types (tu_ctfc);
  output_ctf_strs (tu_ctfc);

  /* The total number of string bytes must be equal to those processed out to
     the str subsection.  */
  gcc_assert (tu_ctfc->ctfc_strlen == get_cur_ctf_str_len (tu_ctfc));

  delete_ctf_container (tu_ctfc);
}

void
ctf_early_global_decl (tree decl)
{
  /* Generate CTF type information if appropriate debug level is set
     to CTFINFO_LEVEL_NORMAL.  */

  if (ctf_debug_info_level == CTFINFO_LEVEL_NORMAL)
    ctf_decl (decl);
}

/* Reset all state within ctfout.c so that we can rerun the compiler
   within the same process.  For use by toplev::finalize.  */

void
ctfout_c_finalize (void)
{
  ctf_info_section = NULL;

  delete_ctf_container (tu_ctfc);
  tu_ctfc = NULL;
}

#include "gt-ctfout.h"
