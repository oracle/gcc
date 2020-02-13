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

/* This file contains implementation of various utility functions to collect
   and keep CTF information.  */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "ctfout.h"

/* Append member definition to the list.  Member list is a singly-linked list
   with list start pointing to the head.  */

void
ctf_dmd_list_append (ctf_dmdef_t ** dmd, ctf_dmdef_t * elem)
{
  ctf_dmdef_t * tail = (dmd && *dmd) ? *dmd : NULL;
  if (tail)
    {
      while (tail->dmd_next)
	tail = tail->dmd_next;

      tail->dmd_next = elem;
    }
  else
    *dmd = elem;

  elem->dmd_next = NULL;
}

/* Compare two CTF variable definition entries.  Currently used for sorting
   by name.  */

int
ctf_varent_compare (const void * entry1, const void * entry2)
{
  int result;
  const ctf_dvdef_t * e1 = *(const ctf_dvdef_t * const*) entry1;
  const ctf_dvdef_t * e2 = *(const ctf_dvdef_t * const*) entry2;

  result = strcmp (e1->dvd_name, e2->dvd_name);

  return result;
}

/* Add str to CTF string table.  No de-duplication of CTF strings is done by
   the compiler.  */

static void
ctfc_strtable_add_str (ctf_strtable_t * str_table, const char * str)
{
  ctf_string_t * ctf_string = ggc_cleared_alloc<ctf_string_t> ();
  /* Keep a reference to the input STR.  */
  ctf_string->cts_str = str;
  ctf_string->cts_next = NULL;

  if (!str_table->ctstab_head)
    str_table->ctstab_head = ctf_string;

  /* Append to the end of the list.  */
  if (str_table->ctstab_tail)
    str_table->ctstab_tail->cts_next = ctf_string;

  str_table->ctstab_tail = ctf_string;
}

const char *
ctf_add_string (ctf_container_ref ctfc, const char * name,
		uint32_t * name_offset)
{
  size_t len;
  char * ctf_string;
  /* Return value is the offset to the string in the string table.  */
  uint32_t str_offset = get_cur_ctf_str_len (ctfc);

  /* Add empty string only once at the beginning of the string table.  Also, do
     not add null strings, return the offset to the empty string for them.  */
  if ((!name || (name != NULL && !strcmp (name, ""))) && str_offset)
    {
      ctf_string = CONST_CAST (char *, ctfc->ctfc_strtable.ctstab_estr);
      str_offset = 0;
    }
  else
    {
      gcc_assert (name);
      /* Add null-terminated strings to the string table.  */
      len = strlen (name) + 1;
      ctf_string = CONST_CAST (char *, ggc_strdup (name));

      ctfc_strtable_add_str (&(ctfc->ctfc_strtable), ctf_string);
      /* Add string to the string table.  Keep number of strings updated.  */
      ctfc->ctfc_strtable.ctstab_num++;
      /* Keep the number of bytes contained in the CTF string table updated.  */
      (ctfc)->ctfc_strtable.ctstab_len += len;
    }

  *name_offset = str_offset;

  return (const char *) ctf_string;
}

/* A CTF type record may be followed by variable-length of bytes to encode the
   CTF type completely.  This routine calculates the number of bytes, in the
   final binary CTF format, which are used to encode information about the type
   completely.

   This function must always be in sync with the CTF header.  */

unsigned long
ctf_calc_num_vbytes (ctf_dtdef_ref ctftype)
{
  uint32_t size;
  unsigned long vlen_bytes = 0;

  uint32_t kind = CTF_V2_INFO_KIND (ctftype->dtd_data.ctti_info);
  uint32_t vlen = CTF_V2_INFO_VLEN (ctftype->dtd_data.ctti_info);

  ctf_dmdef_t * dmd;
  uint32_t size_per_member = 0;
  unsigned int num_members = 0;

  switch (kind)
    {
      case CTF_K_FORWARD:
      case CTF_K_UNKNOWN:
      case CTF_K_POINTER:
      case CTF_K_TYPEDEF:
      case CTF_K_VOLATILE:
      case CTF_K_CONST:
      case CTF_K_RESTRICT:
	/* These types have no vlen data.  */
	break;

      case CTF_K_INTEGER:
      case CTF_K_FLOAT:
	/* 4 bytes to represent encoding CTF_INT_DATA, CTF_FP_DATA.  */
	vlen_bytes += sizeof (uint32_t);
	break;
      case CTF_K_FUNCTION:
	/* FIXME - CTF_PADDING_FOR_ALIGNMENT.  */
	vlen_bytes += (vlen + (vlen & 1)) * sizeof (uint32_t);
	break;
      case CTF_K_ARRAY:
	/* This has a single ctf_array_t.  */
	vlen_bytes += sizeof (ctf_array_t);
	break;
      case CTF_K_SLICE:
	vlen_bytes += sizeof (ctf_slice_t);
	break;
      case CTF_K_STRUCT:
      case CTF_K_UNION:
	/* Count the number and type of members.  */
	size = ctftype->dtd_data.ctti_size;
	size_per_member = size >= CTF_LSTRUCT_THRESH
			  ? sizeof (ctf_lmember_t) : sizeof (ctf_member_t);

	/* Sanity check - number of members of struct must be the same as
	   vlen.  */
	for (dmd = ctftype->dtd_u.dtu_members;
	     dmd != NULL; dmd = (ctf_dmdef_t *) ctf_dmd_list_next (dmd))
	  num_members++;
	gcc_assert (vlen == num_members);

	vlen_bytes += (num_members * size_per_member);
	break;
      case CTF_K_ENUM:
	vlen_bytes += vlen * sizeof (ctf_enum_t);
	break;
      default :
	break;
    }
  return vlen_bytes;
}

void
list_add_ctf_vars (ctf_container_ref ctfc, ctf_dvdef_ref var)
{
  /* FIXME - static may not fly with multiple CUs.  */
  static int num_vars_added = 0;
  ctfc->ctfc_vars_list[num_vars_added++] = var;
}
