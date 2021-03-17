/* Prototypes for functions manipulating DWARF2 DIEs.
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

/* This file contains prototypes for functions defined in dwarf2out.c.  It is
   intended to be included in source files that need some internal knowledge of
   the GCC dwarf structures.  */

#ifndef GCC_DWARF2INT_H
#define GCC_DWARF2INT_H 1

/* Each DIE attribute has a field specifying the attribute kind,
   a link to the next attribute in the chain, and an attribute value.
   Attributes are typically linked below the DIE they modify.  */

typedef struct GTY(()) dw_attr_struct {
  enum dwarf_attribute dw_attr;
  dw_val_node dw_attr_val;
}
dw_attr_node;

extern dw_attr_node *get_AT (dw_die_ref, enum dwarf_attribute);
extern HOST_WIDE_INT AT_int (dw_attr_node *);
extern unsigned HOST_WIDE_INT AT_unsigned (dw_attr_node *a);
extern dw_die_ref get_AT_ref (dw_die_ref, enum dwarf_attribute);
extern const char *get_AT_string (dw_die_ref, enum dwarf_attribute);
extern enum dw_val_class AT_class (dw_attr_node *);
extern unsigned HOST_WIDE_INT AT_unsigned (dw_attr_node *);
extern unsigned get_AT_unsigned (dw_die_ref, enum dwarf_attribute);
extern int get_AT_flag (dw_die_ref, enum dwarf_attribute);

extern void add_name_attribute (dw_die_ref, const char *);

extern dw_die_ref new_die_raw (enum dwarf_tag);
extern dw_die_ref base_type_die (tree, bool);

extern dw_die_ref lookup_decl_die (tree);

extern dw_die_ref dw_get_die_child (dw_die_ref);
extern dw_die_ref dw_get_die_sib (dw_die_ref);
extern enum dwarf_tag dw_get_die_tag (dw_die_ref);

#endif /* !GCC_DWARF2INT_H */
