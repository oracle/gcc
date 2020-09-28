/* ctfout.h - Various declarations for functions found in ctfout.c
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

/* This file defines all data structures used by the compiler to generate
   CTF types.  These are compiler internal representations and closely
   reflect the CTF format requirements in <ctf.h>.  */

#ifndef GCC_CTFOUT_H
#define GCC_CTFOUT_H 1

#include "config.h"
#include "system.h"
#include "tree.h"
#include "fold-const.h"
#include "tree-hash-traits.h"
#include "ctf.h"

/* Invalid CTF type ID definition.  */

#define CTF_NULL_TYPEID 0

/* Value to start generating the CTF type ID from.  */

#define CTF_INIT_TYPEID 1

/* CTF type ID.  */

typedef unsigned long ctf_id_t;

/* CTF string table element (list node).  */

typedef struct GTY ((chain_next ("%h.cts_next"))) ctf_string
{
  const char * cts_str;		  /* CTF string.  */
  struct ctf_string * cts_next;   /* A list node.  */
} ctf_string_t;

/* Internal representation of CTF string table.  */

typedef struct GTY (()) ctf_strtable
{
  ctf_string_t * ctstab_head;	    /* Head str ptr.  */
  ctf_string_t * ctstab_tail;	    /* Tail.  new str appended to tail.  */
  int ctstab_num;		    /* Number of strings in the table.  */
  size_t ctstab_len;		    /* Size of string table in bytes.  */
  const char * ctstab_estr;	    /* Empty string "".  */
} ctf_strtable_t;

/* Encoding information for integers, floating-point values etc.  The flags
   field will contain values appropriate for the type defined in <ctf.h>.  */

typedef struct GTY (()) ctf_encoding
{
  unsigned int cte_format;  /* Data format (CTF_INT_* or CTF_FP_* flags).  */
  unsigned int cte_offset;  /* Offset of value in bits.  */
  unsigned int cte_bits;    /* Size of storage in bits.  */
} ctf_encoding_t;

/* Array information for CTF generation.  */

typedef struct GTY (()) ctf_arinfo
{
  ctf_id_t ctr_contents;	/* Type of array contents.  */
  ctf_id_t ctr_index;		/* Type of array index.  */
  unsigned int ctr_nelems;	/* Number of elements.  */
} ctf_arinfo_t;

/* Function information for CTF generation.  */

typedef struct GTY (()) ctf_funcinfo
{
  ctf_id_t ctc_return;		/* Function return type.  */
  unsigned int ctc_argc;	/* Number of typed arguments to function.  */
  unsigned int ctc_flags;	/* Function attributes (see below).  */
} ctf_funcinfo_t;

typedef struct GTY (()) ctf_sliceinfo
{
  unsigned int cts_type;	/* Reference CTF type.  */
  unsigned short cts_offset;	/* Offset in bits of the first bit.  */
  unsigned short cts_bits;	/* Size in bits.  */
} ctf_sliceinfo_t;

/* CTF type representation internal to the compiler.  It closely reflects the
   ctf_type_t type node in <ctf.h> except the GTY (()) tags.  */

typedef struct GTY (()) ctf_itype
{
  unsigned int ctti_name;	/* Reference to name in string table.  */
  unsigned int ctti_info;	/* Encoded kind, variant length (see below).  */
  union GTY ((desc ("0")))
  {
    unsigned int GTY ((tag ("0"))) _size;/* Size of entire type in bytes.  */
    unsigned int GTY ((tag ("1"))) _type;/* Reference to another type.  */
  } _u;
  unsigned int ctti_lsizehi;	/* High 32 bits of type size in bytes.  */
  unsigned int ctti_lsizelo;	/* Low 32 bits of type size in bytes.  */
} ctf_itype_t;

#define ctti_size _u._size
#define ctti_type _u._type

/* Function arguments end with varargs.  */

#define CTF_FUNC_VARARG 0x1

/* Struct/union/enum member definition for CTF generation.  */

typedef struct GTY ((chain_next ("%h.dmd_next"))) ctf_dmdef
{
  const char * dmd_name;	/* Name of this member.  */
  ctf_id_t dmd_type;		/* Type of this member (for sou).  */
  unsigned int dmd_name_offset;	/* Offset of the name in str table.  */
  unsigned long dmd_offset;	/* Offset of this member in bits (for sou).  */
  int dmd_value;		/* Value of this member (for enum).  */
  struct ctf_dmdef * dmd_next;	/* A list node.  */
} ctf_dmdef_t;

/* Function Argument.  (Encapsulated because GTY machinery does not like
   non struct/union members.  See usage in ctf_dtdef_t.)  */

typedef struct GTY (()) ctf_func_arg
{
  ctf_id_t farg_type;		/* Type identifier of the argument.  */
} ctf_func_arg_t;

typedef struct GTY (()) ctf_dtdef_key
{
  tree dtdk_key_decl;		/* Tree decl corresponding to the type.  */
  unsigned int dtdk_key_flags;  /* Extra flags for hashing the type.  */
} ctf_dtdef_key_t;

/* Type definition for CTF generation.  */

typedef struct GTY (()) ctf_dtdef
{
  ctf_dtdef_key_t dtd_key;    /* Type key for hashing.  */
  const char * dtd_name;      /* Name associated with definition (if any).  */
  ctf_id_t dtd_type;	      /* Type identifier for this definition.  */
  ctf_itype_t dtd_data;	      /* Type node.  */
  union GTY ((desc ("ctf_dtu_d_union_selector (&%1)")))
  {
    /* struct, union, or enum.  */
    ctf_dmdef_t * GTY ((tag ("CTF_DTU_D_MEMBERS"))) dtu_members;
    /* array.  */
    ctf_arinfo_t GTY ((tag ("CTF_DTU_D_ARRAY"))) dtu_arr;
    /* integer or float.  */
    ctf_encoding_t GTY ((tag ("CTF_DTU_D_ENCODING"))) dtu_enc;
    /* function.  */
    ctf_func_arg_t * GTY ((tag ("CTF_DTU_D_ARGUMENTS"))) dtu_argv;
    /* slice.  */
    ctf_sliceinfo_t GTY ((tag ("CTF_DTU_D_SLICE"))) dtu_slice;
  } dtd_u;
} ctf_dtdef_t;

#define dtd_decl dtd_key.dtdk_key_decl
#define dtd_key_flags dtd_key.dtdk_key_flags

/* Variable definition for CTF generation.  */

typedef struct GTY (()) ctf_dvdef
{
  tree dvd_decl;		/* Tree decl corresponding to the variable.  */
  const char * dvd_name;	/* Name associated with variable.  */
  unsigned int dvd_name_offset;	/* Offset of the name in str table.  */
  ctf_id_t dvd_type;		/* Type of variable.  */
} ctf_dvdef_t;

typedef ctf_dvdef_t * ctf_dvdef_ref;
typedef ctf_dtdef_t * ctf_dtdef_ref;

/* Helper enum and api for the GTY machinery to work on union dtu_d.  */

enum ctf_dtu_d_union_enum {
  CTF_DTU_D_MEMBERS,
  CTF_DTU_D_ARRAY,
  CTF_DTU_D_ENCODING,
  CTF_DTU_D_ARGUMENTS,
  CTF_DTU_D_SLICE
};

enum ctf_dtu_d_union_enum
ctf_dtu_d_union_selector (ctf_dtdef_ref);

struct ctf_dtdef_hash : ggc_ptr_hash<ctf_dtdef_t>
{
  typedef ctf_dtdef_ref compare_type;
  static hashval_t hash (ctf_dtdef_ref);
  static bool equal (ctf_dtdef_ref, ctf_dtdef_ref);
};

/* CTF container structure.
   It is the context passed around when generating ctf debug info.  There is
   one container per translation unit.  */

typedef struct GTY (()) ctf_container
{
  /* CTF Preamble.  */
  unsigned short ctfc_magic;
  unsigned char ctfc_version;
  unsigned char ctfc_flags;
  unsigned int ctfc_cuname_offset;

  /* CTF types.  */
  hash_map <ctf_dtdef_hash, ctf_dtdef_ref> * GTY (()) ctfc_types;
  /* CTF variables.  */
  hash_map <tree_decl_hash, ctf_dvdef_ref> * GTY (()) ctfc_vars;
  /* CTF string table.  */
  ctf_strtable_t ctfc_strtable;

  unsigned long ctfc_num_types;
  unsigned long ctfc_num_stypes;
  unsigned long ctfc_num_global_funcs;
  unsigned long ctfc_num_global_objts;

  /* Number of vlen bytes - the variable length portion after ctf_type_t and
     ctf_stype_t in the CTF section.  This is used to calculate the offsets in
     the CTF header.  */
  unsigned long ctfc_num_vlen_bytes;

  /* Next CTF type id to assign.  */
  ctf_id_t ctfc_nextid;
  /* List of pre-processed CTF Variables.  CTF requires that the variables
     appear in the sorted order of their names.  */
  ctf_dvdef_t ** GTY ((length ("%h.ctfc_vars ? %h.ctfc_vars->elements () : 0"))) ctfc_vars_list;
  /* List of pre-processed CTF types.  CTF requires that a shared type must
     appear before the type that uses it.  For the compiler, this means types
     are emitted in sorted order of their type IDs.  */
  ctf_dtdef_t ** GTY ((length ("%h.ctfc_types ? %h.ctfc_types->elements () : 0"))) ctfc_types_list;
  /* List of CTF function types for global functions.  The order of global
     function entries in the CTF funcinfo section is undefined by the
     compiler.  */
  ctf_dtdef_t ** GTY ((length ("%h.ctfc_num_global_funcs"))) ctfc_gfuncs_list;
  /* List of CTF variables at global scope.  The order of global object entries
     in the CTF objinfo section is undefined by the  compiler.  */
  ctf_dvdef_t ** GTY ((length ("%h.ctfc_num_global_objts"))) ctfc_gobjts_list;

  /* Following members are for debugging only.  They do not add functional
     value to the task of CTF creation.  These can be cleaned up once CTF
     generation stabilizes.  */

  /* Keep a count of the number of bytes dumped in asm for debugging
     purposes.  */
  unsigned long ctfc_numbytes_asm;
   /* Total length of all strings in CTF.  */
  size_t ctfc_strlen;

} ctf_container_t;

typedef ctf_container_t * ctf_container_ref;

/* If the next ctf type id is still set to the init value, no ctf records to
   report.  */
#define is_empty_container(ctfc) (((ctfc)->ctfc_nextid == CTF_INIT_TYPEID))
#define get_num_ctf_vars(ctfc)   (ctfc->ctfc_vars->elements ())
#define get_num_ctf_types(ctfc)  (ctfc->ctfc_types->elements ())

#define get_cur_ctf_str_len(ctfc)      ((ctfc)->ctfc_strtable.ctstab_len)

#define get_ctfc_num_vlen_bytes(ctfc)	((ctfc)->ctfc_num_vlen_bytes)

void ctf_debug_init (void);

void ctf_early_global_decl (tree decl);

void ctf_early_finish (const char * filename);

void ctfout_c_finalize (void);

/* The compiler demarcates whether types are visible at top-level scope or not.
   The only example so far of a type not visible at top-level scope is slices.
   CTF_ADD_NONROOT is used to indicate the latter.  */
#define	CTF_ADD_NONROOT	0	/* CTF type only visible in nested scope.  */
#define	CTF_ADD_ROOT	1	/* CTF type visible at top-level scope.  */

/* Interface from ctfcreate.c to ctfout.c.
   These APIs create CTF types and add them to the CTF container associated
   with the translation unit.  The return value is the typeID of the CTF type
   added to the container.  */
extern ctf_id_t ctf_add_volatile (ctf_container_ref, uint32_t, ctf_id_t, tree,
				  uint32_t);
extern ctf_id_t ctf_add_const (ctf_container_ref, uint32_t, ctf_id_t, tree,
			       uint32_t);
extern ctf_id_t ctf_add_restrict (ctf_container_ref, uint32_t, ctf_id_t, tree,
				  uint32_t);
extern ctf_id_t ctf_add_enum (ctf_container_ref, uint32_t, const char *,
			      HOST_WIDE_INT, tree);
extern ctf_id_t ctf_add_slice (ctf_container_ref, uint32_t, ctf_id_t,
			       const ctf_encoding_t *, tree);
extern ctf_id_t ctf_add_float (ctf_container_ref, uint32_t, const char *,
			       const ctf_encoding_t *, tree);
extern ctf_id_t ctf_add_integer (ctf_container_ref, uint32_t, const char *,
				 const ctf_encoding_t *, tree);
extern ctf_id_t ctf_add_pointer (ctf_container_ref, uint32_t flag, ctf_id_t,
				 tree);
extern ctf_id_t ctf_add_array (ctf_container_ref, uint32_t,
			       const ctf_arinfo_t *, tree);
extern ctf_id_t ctf_add_forward (ctf_container_ref, uint32_t, const char *,
				 uint32_t, tree);
extern ctf_id_t ctf_add_typedef (ctf_container_ref, uint32_t, const char *,
				 ctf_id_t, tree);
extern ctf_id_t ctf_add_function (ctf_container_ref, uint32_t, const char *,
				  const ctf_funcinfo_t *, ctf_func_arg_t *,
				  tree);
extern ctf_id_t ctf_add_sou (ctf_container_ref, uint32_t, const char *,
			     uint32_t, size_t, tree);

extern int ctf_add_enumerator (ctf_container_ref, ctf_id_t, const char *,
			       HOST_WIDE_INT, tree);
extern int ctf_add_member_offset (ctf_container_ref, tree, const char *,
				  ctf_id_t, unsigned long);
extern int ctf_add_variable (ctf_container_ref, const char *, ctf_id_t, tree);

/* Interface from ctfutils.c.
   Utility functions for CTF generation.  */

#define ctf_dmd_list_next(elem)		((ctf_dmdef_t *)((elem)->dmd_next))

extern void ctf_dmd_list_append (ctf_dmdef_t **, ctf_dmdef_t *);

extern void ctf_dtd_insert (ctf_container_ref, ctf_dtdef_ref);
extern void ctf_dtd_delete (ctf_container_ref, ctf_dtdef_ref);
extern ctf_dtdef_ref ctf_dtd_lookup (const ctf_container_ref, const tree);
extern ctf_dtdef_ref ctf_dtd_lookup_with_flags (const ctf_container_ref,
						const tree,
						const unsigned int);

extern void ctf_dvd_insert (ctf_container_ref, ctf_dvdef_ref);
extern void ctf_dvd_delete (ctf_container_ref, ctf_dvdef_ref);
extern ctf_dvdef_ref ctf_dvd_lookup (const ctf_container_ref, const tree);

extern int ctf_varent_compare (const void *, const void *);
extern unsigned long ctf_calc_num_vbytes (ctf_dtdef_ref);

/* Add a str to the CTF string table.  */
extern const char * ctf_add_string (ctf_container_ref, const char *,
				    uint32_t *);

extern void list_add_ctf_vars (ctf_container_ref, ctf_dvdef_ref);

/* Interface from ctfout.c to ctfutils.c.  */

extern hashval_t hash_dtd_tree_decl (tree, uint32_t);
extern hashval_t hash_dvd_tree_decl (tree);

#endif /* GCC_CTFOUT_H */
