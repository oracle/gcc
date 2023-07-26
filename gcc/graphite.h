/* Graphite polyhedral representation.
   Copyright (C) 2009-2018 Free Software Foundation, Inc.
   Contributed by Sebastian Pop <sebastian.pop@amd.com> and
   Tobias Grosser <grosser@fim.uni-passau.de>.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

#ifndef GCC_GRAPHITE_POLY_H
#define GCC_GRAPHITE_POLY_H

#include "sese.h"
#include <isl/options.h>
#include <isl/ctx.h>
#include <isl/val.h>
#include <isl/set.h>
#include <isl/union_set.h>
#include <isl/map.h>
#include <isl/union_map.h>
#include <isl/aff.h>
#include <isl/constraint.h>
#include <isl/flow.h>
#include <isl/ilp.h>
#include <isl/schedule.h>
#include <isl/ast_build.h>
#include <isl/schedule_node.h>
#include <isl/id.h>
#include <isl/space.h>
#include <isl/version.h>
#include <dlfcn.h>

#define DYNSYMS \
  DYNSYM (isl_aff_add_coefficient_si); \
  DYNSYM (isl_aff_free); \
  DYNSYM (isl_aff_get_space); \
  DYNSYM (isl_aff_set_coefficient_si); \
  DYNSYM (isl_aff_set_constant_si); \
  DYNSYM (isl_aff_zero_on_domain); \
  DYNSYM (isl_band_free); \
  DYNSYM (isl_band_get_children); \
  DYNSYM (isl_band_get_partial_schedule); \
  DYNSYM (isl_band_has_children); \
  DYNSYM (isl_band_list_free); \
  DYNSYM (isl_band_list_get_band); \
  DYNSYM (isl_band_list_get_ctx); \
  DYNSYM (isl_band_list_n_band); \
  DYNSYM (isl_band_n_member); \
  DYNSYM (isl_basic_map_add_constraint); \
  DYNSYM (isl_basic_map_project_out); \
  DYNSYM (isl_basic_map_universe); \
  DYNSYM (isl_constraint_set_coefficient_si); \
  DYNSYM (isl_constraint_set_constant_si); \
  DYNSYM (isl_ctx_alloc); \
  DYNSYM (isl_ctx_free); \
  DYNSYM (isl_equality_alloc); \
  DYNSYM (isl_id_alloc); \
  DYNSYM (isl_id_copy); \
  DYNSYM (isl_id_free); \
  DYNSYM (isl_inequality_alloc); \
  DYNSYM (isl_local_space_copy); \
  DYNSYM (isl_local_space_free); \
  DYNSYM (isl_local_space_from_space); \
  DYNSYM (isl_local_space_range); \
  DYNSYM (isl_map_add_constraint); \
  DYNSYM (isl_map_add_dims); \
  DYNSYM (isl_map_align_params); \
  DYNSYM (isl_map_apply_range); \
  DYNSYM (isl_map_copy); \
  DYNSYM (isl_map_dim); \
  DYNSYM (isl_map_dump); \
  DYNSYM (isl_map_equate); \
  DYNSYM (isl_map_fix_si); \
  DYNSYM (isl_map_flat_product); \
  DYNSYM (isl_map_flat_range_product); \
  DYNSYM (isl_map_free); \
  DYNSYM (isl_map_from_basic_map); \
  DYNSYM (isl_map_from_pw_aff); \
  DYNSYM (isl_map_from_union_map); \
  DYNSYM (isl_map_get_ctx); \
  DYNSYM (isl_map_get_space); \
  DYNSYM (isl_map_get_tuple_id); \
  DYNSYM (isl_map_insert_dims); \
  DYNSYM (isl_map_intersect); \
  DYNSYM (isl_map_intersect_domain); \
  DYNSYM (isl_map_intersect_range); \
  DYNSYM (isl_map_is_empty); \
  DYNSYM (isl_map_lex_ge); \
  DYNSYM (isl_map_lex_le); \
  DYNSYM (isl_map_n_out); \
  DYNSYM (isl_map_range); \
  DYNSYM (isl_map_set_tuple_id); \
  DYNSYM (isl_map_universe); \
  DYNSYM (isl_options_set_on_error); \
  DYNSYM (isl_options_set_schedule_serialize_sccs); \
  DYNSYM (isl_printer_set_yaml_style); \
  DYNSYM (isl_options_set_schedule_max_constant_term); \
  DYNSYM (isl_options_set_schedule_maximize_band_depth); \
  DYNSYM (isl_printer_free); \
  DYNSYM (isl_printer_print_aff); \
  DYNSYM (isl_printer_print_constraint); \
  DYNSYM (isl_printer_print_map); \
  DYNSYM (isl_printer_print_set); \
  DYNSYM (isl_printer_to_file); \
  DYNSYM (isl_pw_aff_add); \
  DYNSYM (isl_pw_aff_alloc); \
  DYNSYM (isl_pw_aff_copy); \
  DYNSYM (isl_pw_aff_eq_set); \
  DYNSYM (isl_pw_aff_free); \
  DYNSYM (isl_pw_aff_from_aff); \
  DYNSYM (isl_pw_aff_ge_set); \
  DYNSYM (isl_pw_aff_gt_set); \
  DYNSYM (isl_pw_aff_is_cst); \
  DYNSYM (isl_pw_aff_le_set); \
  DYNSYM (isl_pw_aff_lt_set); \
  DYNSYM (isl_pw_aff_mul); \
  DYNSYM (isl_pw_aff_ne_set); \
  DYNSYM (isl_pw_aff_nonneg_set); \
  DYNSYM (isl_pw_aff_set_tuple_id); \
  DYNSYM (isl_pw_aff_sub); \
  DYNSYM (isl_pw_aff_zero_set); \
  DYNSYM (isl_schedule_free); \
  DYNSYM (isl_schedule_get_band_forest); \
  DYNSYM (isl_set_add_constraint); \
  DYNSYM (isl_set_add_dims); \
  DYNSYM (isl_set_apply); \
  DYNSYM (isl_set_coalesce); \
  DYNSYM (isl_set_copy); \
  DYNSYM (isl_set_dim); \
  DYNSYM (isl_set_fix_si); \
  DYNSYM (isl_set_free); \
  DYNSYM (isl_set_get_space); \
  DYNSYM (isl_set_get_tuple_id); \
  DYNSYM (isl_set_intersect); \
  DYNSYM (isl_set_is_empty); \
  DYNSYM (isl_set_n_dim); \
  DYNSYM (isl_set_nat_universe); \
  DYNSYM (isl_set_project_out); \
  DYNSYM (isl_set_set_tuple_id); \
  DYNSYM (isl_set_universe); \
  DYNSYM (isl_space_add_dims); \
  DYNSYM (isl_space_alloc); \
  DYNSYM (isl_space_copy); \
  DYNSYM (isl_space_dim); \
  DYNSYM (isl_space_domain); \
  DYNSYM (isl_space_find_dim_by_id); \
  DYNSYM (isl_space_free); \
  DYNSYM (isl_space_from_domain); \
  DYNSYM (isl_space_get_tuple_id); \
  DYNSYM (isl_space_params_alloc); \
  DYNSYM (isl_space_range); \
  DYNSYM (isl_space_set_alloc); \
  DYNSYM (isl_space_set_dim_id); \
  DYNSYM (isl_space_set_tuple_id); \
  DYNSYM (isl_union_map_add_map); \
  DYNSYM (isl_union_map_align_params); \
  DYNSYM (isl_union_map_apply_domain); \
  DYNSYM (isl_union_map_apply_range); \
  DYNSYM (isl_union_map_compute_flow); \
  DYNSYM (isl_union_map_copy); \
  DYNSYM (isl_union_map_empty); \
  DYNSYM (isl_union_map_flat_range_product); \
  DYNSYM (isl_union_map_foreach_map); \
  DYNSYM (isl_union_map_free); \
  DYNSYM (isl_union_map_from_map); \
  DYNSYM (isl_union_map_get_ctx); \
  DYNSYM (isl_union_map_get_space); \
  DYNSYM (isl_union_map_gist_domain); \
  DYNSYM (isl_union_map_gist_range); \
  DYNSYM (isl_union_map_intersect_domain); \
  DYNSYM (isl_union_map_is_empty); \
  DYNSYM (isl_union_map_subtract); \
  DYNSYM (isl_union_map_union); \
  DYNSYM (isl_union_set_add_set); \
  DYNSYM (isl_union_set_compute_schedule); \
  DYNSYM (isl_union_set_copy); \
  DYNSYM (isl_union_set_empty); \
  DYNSYM (isl_union_set_from_set); \
  DYNSYM (isl_aff_add_constant_val); \
  DYNSYM (isl_aff_get_coefficient_val); \
  DYNSYM (isl_aff_get_ctx); \
  DYNSYM (isl_aff_mod_val); \
  DYNSYM (isl_ast_build_ast_from_schedule); \
  DYNSYM (isl_ast_build_free); \
  DYNSYM (isl_ast_build_from_context); \
  DYNSYM (isl_ast_build_get_ctx); \
  DYNSYM (isl_ast_build_get_schedule); \
  DYNSYM (isl_ast_build_get_schedule_space); \
  DYNSYM (isl_ast_build_set_before_each_for); \
  DYNSYM (isl_ast_build_set_options); \
  DYNSYM (isl_ast_expr_free); \
  DYNSYM (isl_ast_expr_from_val); \
  DYNSYM (isl_ast_expr_get_ctx); \
  DYNSYM (isl_ast_expr_get_id); \
  DYNSYM (isl_ast_expr_get_op_arg); \
  DYNSYM (isl_ast_expr_get_op_n_arg); \
  DYNSYM (isl_ast_expr_get_op_type); \
  DYNSYM (isl_ast_expr_get_type); \
  DYNSYM (isl_ast_expr_get_val); \
  DYNSYM (isl_ast_expr_sub); \
  DYNSYM (isl_ast_node_block_get_children); \
  DYNSYM (isl_ast_node_for_get_body); \
  DYNSYM (isl_ast_node_for_get_cond); \
  DYNSYM (isl_ast_node_for_get_inc); \
  DYNSYM (isl_ast_node_for_get_init); \
  DYNSYM (isl_ast_node_for_get_iterator); \
  DYNSYM (isl_ast_node_free); \
  DYNSYM (isl_ast_node_get_annotation); \
  DYNSYM (isl_ast_node_get_type); \
  DYNSYM (isl_ast_node_if_get_cond); \
  DYNSYM (isl_ast_node_if_get_else); \
  DYNSYM (isl_ast_node_if_get_then); \
  DYNSYM (isl_ast_node_list_free); \
  DYNSYM (isl_ast_node_list_get_ast_node); \
  DYNSYM (isl_ast_node_list_n_ast_node); \
  DYNSYM (isl_ast_node_user_get_expr); \
  DYNSYM (isl_constraint_set_coefficient_val); \
  DYNSYM (isl_constraint_set_constant_val); \
  DYNSYM (isl_id_get_user); \
  DYNSYM (isl_local_space_get_ctx); \
  DYNSYM (isl_map_fix_val); \
  DYNSYM (isl_options_set_ast_build_atomic_upper_bound); \
  DYNSYM (isl_printer_print_ast_node); \
  DYNSYM (isl_printer_print_str); \
  DYNSYM (isl_printer_set_output_format); \
  DYNSYM (isl_pw_aff_mod_val); \
  DYNSYM (isl_schedule_constraints_compute_schedule); \
  DYNSYM (isl_schedule_constraints_on_domain); \
  DYNSYM (isl_schedule_constraints_set_coincidence); \
  DYNSYM (isl_schedule_constraints_set_proximity); \
  DYNSYM (isl_schedule_constraints_set_validity); \
  DYNSYM (isl_set_get_dim_id); \
  DYNSYM (isl_set_max_val); \
  DYNSYM (isl_set_min_val); \
  DYNSYM (isl_set_params); \
  DYNSYM (isl_space_align_params); \
  DYNSYM (isl_space_map_from_domain_and_range); \
  DYNSYM (isl_space_set_tuple_name); \
  DYNSYM (isl_space_wrap); \
  DYNSYM (isl_union_map_from_domain_and_range); \
  DYNSYM (isl_union_map_range); \
  DYNSYM (isl_union_set_union); \
  DYNSYM (isl_union_set_universe); \
  DYNSYM (isl_val_2exp); \
  DYNSYM (isl_val_add_ui); \
  DYNSYM (isl_val_copy); \
  DYNSYM (isl_val_free); \
  DYNSYM (isl_val_int_from_si); \
  DYNSYM (isl_val_int_from_ui); \
  DYNSYM (isl_val_mul); \
  DYNSYM (isl_val_neg); \
  DYNSYM (isl_val_sub); \
  DYNSYM (isl_printer_print_union_map); \
  DYNSYM (isl_pw_aff_get_ctx); \
  DYNSYM (isl_val_is_int); \
  DYNSYM (isl_ctx_get_max_operations); \
  DYNSYM (isl_ctx_set_max_operations); \
  DYNSYM (isl_ctx_last_error); \
  DYNSYM (isl_ctx_reset_operations); \
  DYNSYM (isl_map_coalesce); \
  DYNSYM (isl_printer_print_schedule); \
  DYNSYM (isl_set_set_dim_id); \
  DYNSYM (isl_union_map_coalesce); \
  DYNSYM (isl_multi_val_set_val); \
  DYNSYM (isl_multi_val_zero); \
  DYNSYM (isl_options_set_schedule_max_coefficient); \
  DYNSYM (isl_options_set_tile_scale_tile_loops); \
  DYNSYM (isl_schedule_copy); \
  DYNSYM (isl_schedule_get_map); \
  DYNSYM (isl_schedule_map_schedule_node_bottom_up); \
  DYNSYM (isl_schedule_node_band_get_permutable); \
  DYNSYM (isl_schedule_node_band_get_space); \
  DYNSYM (isl_schedule_node_band_tile); \
  DYNSYM (isl_schedule_node_child); \
  DYNSYM (isl_schedule_node_free); \
  DYNSYM (isl_schedule_node_get_child); \
  DYNSYM (isl_schedule_node_get_ctx); \
  DYNSYM (isl_schedule_node_get_type); \
  DYNSYM (isl_schedule_node_n_children); \
  DYNSYM (isl_union_map_is_equal); \
  DYNSYM (isl_union_access_info_compute_flow); \
  DYNSYM (isl_union_access_info_from_sink); \
  DYNSYM (isl_union_access_info_set_may_source); \
  DYNSYM (isl_union_access_info_set_must_source); \
  DYNSYM (isl_union_access_info_set_schedule); \
  DYNSYM (isl_union_flow_free); \
  DYNSYM (isl_union_flow_get_may_dependence); \
  DYNSYM (isl_union_flow_get_must_dependence); \
  DYNSYM (isl_aff_var_on_domain); \
  DYNSYM (isl_multi_aff_from_aff); \
  DYNSYM (isl_schedule_get_ctx); \
  DYNSYM (isl_multi_aff_set_tuple_id); \
  DYNSYM (isl_multi_aff_dim); \
  DYNSYM (isl_schedule_get_domain); \
  DYNSYM (isl_union_set_is_empty); \
  DYNSYM (isl_union_set_get_space); \
  DYNSYM (isl_union_pw_multi_aff_empty); \
  DYNSYM (isl_union_set_foreach_set); \
  DYNSYM (isl_union_set_free); \
  DYNSYM (isl_multi_union_pw_aff_from_union_pw_multi_aff); \
  DYNSYM (isl_multi_union_pw_aff_apply_multi_aff); \
  DYNSYM (isl_schedule_insert_partial_schedule); \
  DYNSYM (isl_union_pw_multi_aff_free); \
  DYNSYM (isl_pw_multi_aff_project_out_map); \
  DYNSYM (isl_union_pw_multi_aff_add_pw_multi_aff); \
  DYNSYM (isl_schedule_from_domain); \
  DYNSYM (isl_schedule_sequence); \
  DYNSYM (isl_ast_build_node_from_schedule); \
  DYNSYM (isl_ast_node_mark_get_node); \
  DYNSYM (isl_schedule_node_band_member_get_ast_loop_type); \
  DYNSYM (isl_schedule_node_band_member_set_ast_loop_type); \
  DYNSYM (isl_val_n_abs_num_chunks); \
  DYNSYM (isl_val_get_abs_num_chunks); \
  DYNSYM (isl_val_int_from_chunks); \
  DYNSYM (isl_val_is_neg); \
  DYNSYM (isl_version); \
  DYNSYM (isl_options_get_on_error); \
  DYNSYM (isl_ctx_reset_error);

extern struct isl_pointers_s__
{
  bool inited;
  void *h;
#define DYNSYM(x) __typeof (x) *p_##x
  DYNSYMS
#undef DYNSYM
} isl_pointers__;

#define isl_aff_add_coefficient_si (*isl_pointers__.p_isl_aff_add_coefficient_si)
#define isl_aff_free (*isl_pointers__.p_isl_aff_free)
#define isl_aff_get_space (*isl_pointers__.p_isl_aff_get_space)
#define isl_aff_set_coefficient_si (*isl_pointers__.p_isl_aff_set_coefficient_si)
#define isl_aff_set_constant_si (*isl_pointers__.p_isl_aff_set_constant_si)
#define isl_aff_zero_on_domain (*isl_pointers__.p_isl_aff_zero_on_domain)
#define isl_band_free (*isl_pointers__.p_isl_band_free)
#define isl_band_get_children (*isl_pointers__.p_isl_band_get_children)
#define isl_band_get_partial_schedule (*isl_pointers__.p_isl_band_get_partial_schedule)
#define isl_band_has_children (*isl_pointers__.p_isl_band_has_children)
#define isl_band_list_free (*isl_pointers__.p_isl_band_list_free)
#define isl_band_list_get_band (*isl_pointers__.p_isl_band_list_get_band)
#define isl_band_list_get_ctx (*isl_pointers__.p_isl_band_list_get_ctx)
#define isl_band_list_n_band (*isl_pointers__.p_isl_band_list_n_band)
#define isl_band_n_member (*isl_pointers__.p_isl_band_n_member)
#define isl_basic_map_add_constraint (*isl_pointers__.p_isl_basic_map_add_constraint)
#define isl_basic_map_project_out (*isl_pointers__.p_isl_basic_map_project_out)
#define isl_basic_map_universe (*isl_pointers__.p_isl_basic_map_universe)
#define isl_constraint_set_coefficient_si (*isl_pointers__.p_isl_constraint_set_coefficient_si)
#define isl_constraint_set_constant_si (*isl_pointers__.p_isl_constraint_set_constant_si)
#define isl_ctx_alloc (*isl_pointers__.p_isl_ctx_alloc)
#define isl_ctx_free (*isl_pointers__.p_isl_ctx_free)
#define isl_equality_alloc (*isl_pointers__.p_isl_equality_alloc)
#define isl_id_alloc (*isl_pointers__.p_isl_id_alloc)
#define isl_id_copy (*isl_pointers__.p_isl_id_copy)
#define isl_id_free (*isl_pointers__.p_isl_id_free)
#define isl_inequality_alloc (*isl_pointers__.p_isl_inequality_alloc)
#define isl_local_space_copy (*isl_pointers__.p_isl_local_space_copy)
#define isl_local_space_free (*isl_pointers__.p_isl_local_space_free)
#define isl_local_space_from_space (*isl_pointers__.p_isl_local_space_from_space)
#define isl_local_space_range (*isl_pointers__.p_isl_local_space_range)
#define isl_map_add_constraint (*isl_pointers__.p_isl_map_add_constraint)
#define isl_map_add_dims (*isl_pointers__.p_isl_map_add_dims)
#define isl_map_align_params (*isl_pointers__.p_isl_map_align_params)
#define isl_map_apply_range (*isl_pointers__.p_isl_map_apply_range)
#define isl_map_copy (*isl_pointers__.p_isl_map_copy)
#define isl_map_dim (*isl_pointers__.p_isl_map_dim)
#define isl_map_dump (*isl_pointers__.p_isl_map_dump)
#define isl_map_equate (*isl_pointers__.p_isl_map_equate)
#define isl_map_fix_si (*isl_pointers__.p_isl_map_fix_si)
#define isl_map_flat_product (*isl_pointers__.p_isl_map_flat_product)
#define isl_map_flat_range_product (*isl_pointers__.p_isl_map_flat_range_product)
#define isl_map_free (*isl_pointers__.p_isl_map_free)
#define isl_map_from_basic_map (*isl_pointers__.p_isl_map_from_basic_map)
#define isl_map_from_pw_aff (*isl_pointers__.p_isl_map_from_pw_aff)
#define isl_map_from_union_map (*isl_pointers__.p_isl_map_from_union_map)
#define isl_map_get_ctx (*isl_pointers__.p_isl_map_get_ctx)
#define isl_map_get_space (*isl_pointers__.p_isl_map_get_space)
#define isl_map_get_tuple_id (*isl_pointers__.p_isl_map_get_tuple_id)
#define isl_map_insert_dims (*isl_pointers__.p_isl_map_insert_dims)
#define isl_map_intersect (*isl_pointers__.p_isl_map_intersect)
#define isl_map_intersect_domain (*isl_pointers__.p_isl_map_intersect_domain)
#define isl_map_intersect_range (*isl_pointers__.p_isl_map_intersect_range)
#define isl_map_is_empty (*isl_pointers__.p_isl_map_is_empty)
#define isl_map_lex_ge (*isl_pointers__.p_isl_map_lex_ge)
#define isl_map_lex_le (*isl_pointers__.p_isl_map_lex_le)
#define isl_map_n_out (*isl_pointers__.p_isl_map_n_out)
#define isl_map_range (*isl_pointers__.p_isl_map_range)
#define isl_map_set_tuple_id (*isl_pointers__.p_isl_map_set_tuple_id)
#define isl_map_universe (*isl_pointers__.p_isl_map_universe)
#define isl_options_set_on_error (*isl_pointers__.p_isl_options_set_on_error)
#define isl_options_set_schedule_serialize_sccs (*isl_pointers__.p_isl_options_set_schedule_serialize_sccs)
#define isl_printer_set_yaml_style (*isl_pointers__.p_isl_printer_set_yaml_style)
#define isl_options_set_schedule_max_constant_term (*isl_pointers__.p_isl_options_set_schedule_max_constant_term)
#define isl_options_set_schedule_maximize_band_depth (*isl_pointers__.p_isl_options_set_schedule_maximize_band_depth)
#define isl_printer_free (*isl_pointers__.p_isl_printer_free)
#define isl_printer_print_aff (*isl_pointers__.p_isl_printer_print_aff)
#define isl_printer_print_constraint (*isl_pointers__.p_isl_printer_print_constraint)
#define isl_printer_print_map (*isl_pointers__.p_isl_printer_print_map)
#define isl_printer_print_set (*isl_pointers__.p_isl_printer_print_set)
#define isl_printer_to_file (*isl_pointers__.p_isl_printer_to_file)
#define isl_pw_aff_add (*isl_pointers__.p_isl_pw_aff_add)
#define isl_pw_aff_alloc (*isl_pointers__.p_isl_pw_aff_alloc)
#define isl_pw_aff_copy (*isl_pointers__.p_isl_pw_aff_copy)
#define isl_pw_aff_eq_set (*isl_pointers__.p_isl_pw_aff_eq_set)
#define isl_pw_aff_free (*isl_pointers__.p_isl_pw_aff_free)
#define isl_pw_aff_from_aff (*isl_pointers__.p_isl_pw_aff_from_aff)
#define isl_pw_aff_ge_set (*isl_pointers__.p_isl_pw_aff_ge_set)
#define isl_pw_aff_gt_set (*isl_pointers__.p_isl_pw_aff_gt_set)
#define isl_pw_aff_is_cst (*isl_pointers__.p_isl_pw_aff_is_cst)
#define isl_pw_aff_le_set (*isl_pointers__.p_isl_pw_aff_le_set)
#define isl_pw_aff_lt_set (*isl_pointers__.p_isl_pw_aff_lt_set)
#define isl_pw_aff_mul (*isl_pointers__.p_isl_pw_aff_mul)
#define isl_pw_aff_ne_set (*isl_pointers__.p_isl_pw_aff_ne_set)
#define isl_pw_aff_nonneg_set (*isl_pointers__.p_isl_pw_aff_nonneg_set)
#define isl_pw_aff_set_tuple_id (*isl_pointers__.p_isl_pw_aff_set_tuple_id)
#define isl_pw_aff_sub (*isl_pointers__.p_isl_pw_aff_sub)
#define isl_pw_aff_zero_set (*isl_pointers__.p_isl_pw_aff_zero_set)
#define isl_schedule_free (*isl_pointers__.p_isl_schedule_free)
#define isl_schedule_get_band_forest (*isl_pointers__.p_isl_schedule_get_band_forest)
#define isl_set_add_constraint (*isl_pointers__.p_isl_set_add_constraint)
#define isl_set_add_dims (*isl_pointers__.p_isl_set_add_dims)
#define isl_set_apply (*isl_pointers__.p_isl_set_apply)
#define isl_set_coalesce (*isl_pointers__.p_isl_set_coalesce)
#define isl_set_copy (*isl_pointers__.p_isl_set_copy)
#define isl_set_dim (*isl_pointers__.p_isl_set_dim)
#define isl_set_fix_si (*isl_pointers__.p_isl_set_fix_si)
#define isl_set_free (*isl_pointers__.p_isl_set_free)
#define isl_set_get_space (*isl_pointers__.p_isl_set_get_space)
#define isl_set_get_tuple_id (*isl_pointers__.p_isl_set_get_tuple_id)
#define isl_set_intersect (*isl_pointers__.p_isl_set_intersect)
#define isl_set_is_empty (*isl_pointers__.p_isl_set_is_empty)
#define isl_set_n_dim (*isl_pointers__.p_isl_set_n_dim)
#define isl_set_nat_universe (*isl_pointers__.p_isl_set_nat_universe)
#define isl_set_project_out (*isl_pointers__.p_isl_set_project_out)
#define isl_set_set_tuple_id (*isl_pointers__.p_isl_set_set_tuple_id)
#define isl_set_universe (*isl_pointers__.p_isl_set_universe)
#define isl_space_add_dims (*isl_pointers__.p_isl_space_add_dims)
#define isl_space_alloc (*isl_pointers__.p_isl_space_alloc)
#define isl_space_copy (*isl_pointers__.p_isl_space_copy)
#define isl_space_dim (*isl_pointers__.p_isl_space_dim)
#define isl_space_domain (*isl_pointers__.p_isl_space_domain)
#define isl_space_find_dim_by_id (*isl_pointers__.p_isl_space_find_dim_by_id)
#define isl_space_free (*isl_pointers__.p_isl_space_free)
#define isl_space_from_domain (*isl_pointers__.p_isl_space_from_domain)
#define isl_space_get_tuple_id (*isl_pointers__.p_isl_space_get_tuple_id)
#define isl_space_params_alloc (*isl_pointers__.p_isl_space_params_alloc)
#define isl_space_range (*isl_pointers__.p_isl_space_range)
#define isl_space_set_alloc (*isl_pointers__.p_isl_space_set_alloc)
#define isl_space_set_dim_id (*isl_pointers__.p_isl_space_set_dim_id)
#define isl_space_set_tuple_id (*isl_pointers__.p_isl_space_set_tuple_id)
#define isl_union_map_add_map (*isl_pointers__.p_isl_union_map_add_map)
#define isl_union_map_align_params (*isl_pointers__.p_isl_union_map_align_params)
#define isl_union_map_apply_domain (*isl_pointers__.p_isl_union_map_apply_domain)
#define isl_union_map_apply_range (*isl_pointers__.p_isl_union_map_apply_range)
#define isl_union_map_compute_flow (*isl_pointers__.p_isl_union_map_compute_flow)
#define isl_union_map_copy (*isl_pointers__.p_isl_union_map_copy)
#define isl_union_map_empty (*isl_pointers__.p_isl_union_map_empty)
#define isl_union_map_flat_range_product (*isl_pointers__.p_isl_union_map_flat_range_product)
#define isl_union_map_foreach_map (*isl_pointers__.p_isl_union_map_foreach_map)
#define isl_union_map_free (*isl_pointers__.p_isl_union_map_free)
#define isl_union_map_from_map (*isl_pointers__.p_isl_union_map_from_map)
#define isl_union_map_get_ctx (*isl_pointers__.p_isl_union_map_get_ctx)
#define isl_union_map_get_space (*isl_pointers__.p_isl_union_map_get_space)
#define isl_union_map_gist_domain (*isl_pointers__.p_isl_union_map_gist_domain)
#define isl_union_map_gist_range (*isl_pointers__.p_isl_union_map_gist_range)
#define isl_union_map_intersect_domain (*isl_pointers__.p_isl_union_map_intersect_domain)
#define isl_union_map_is_empty (*isl_pointers__.p_isl_union_map_is_empty)
#define isl_union_map_subtract (*isl_pointers__.p_isl_union_map_subtract)
#define isl_union_map_union (*isl_pointers__.p_isl_union_map_union)
#define isl_union_set_add_set (*isl_pointers__.p_isl_union_set_add_set)
#define isl_union_set_compute_schedule (*isl_pointers__.p_isl_union_set_compute_schedule)
#define isl_union_set_copy (*isl_pointers__.p_isl_union_set_copy)
#define isl_union_set_empty (*isl_pointers__.p_isl_union_set_empty)
#define isl_union_set_from_set (*isl_pointers__.p_isl_union_set_from_set)
#define isl_aff_add_constant_val (*isl_pointers__.p_isl_aff_add_constant_val)
#define isl_aff_get_coefficient_val (*isl_pointers__.p_isl_aff_get_coefficient_val)
#define isl_aff_get_ctx (*isl_pointers__.p_isl_aff_get_ctx)
#define isl_aff_mod_val (*isl_pointers__.p_isl_aff_mod_val)
#define isl_ast_build_ast_from_schedule (*isl_pointers__.p_isl_ast_build_ast_from_schedule)
#define isl_ast_build_free (*isl_pointers__.p_isl_ast_build_free)
#define isl_ast_build_from_context (*isl_pointers__.p_isl_ast_build_from_context)
#define isl_ast_build_get_ctx (*isl_pointers__.p_isl_ast_build_get_ctx)
#define isl_ast_build_get_schedule (*isl_pointers__.p_isl_ast_build_get_schedule)
#define isl_ast_build_get_schedule_space (*isl_pointers__.p_isl_ast_build_get_schedule_space)
#define isl_ast_build_set_before_each_for (*isl_pointers__.p_isl_ast_build_set_before_each_for)
#define isl_ast_build_set_options (*isl_pointers__.p_isl_ast_build_set_options)
#define isl_ast_expr_free (*isl_pointers__.p_isl_ast_expr_free)
#define isl_ast_expr_from_val (*isl_pointers__.p_isl_ast_expr_from_val)
#define isl_ast_expr_get_ctx (*isl_pointers__.p_isl_ast_expr_get_ctx)
#define isl_ast_expr_get_id (*isl_pointers__.p_isl_ast_expr_get_id)
#define isl_ast_expr_get_op_arg (*isl_pointers__.p_isl_ast_expr_get_op_arg)
#define isl_ast_expr_get_op_n_arg (*isl_pointers__.p_isl_ast_expr_get_op_n_arg)
#define isl_ast_expr_get_op_type (*isl_pointers__.p_isl_ast_expr_get_op_type)
#define isl_ast_expr_get_type (*isl_pointers__.p_isl_ast_expr_get_type)
#define isl_ast_expr_get_val (*isl_pointers__.p_isl_ast_expr_get_val)
#define isl_ast_expr_sub (*isl_pointers__.p_isl_ast_expr_sub)
#define isl_ast_node_block_get_children (*isl_pointers__.p_isl_ast_node_block_get_children)
#define isl_ast_node_for_get_body (*isl_pointers__.p_isl_ast_node_for_get_body)
#define isl_ast_node_for_get_cond (*isl_pointers__.p_isl_ast_node_for_get_cond)
#define isl_ast_node_for_get_inc (*isl_pointers__.p_isl_ast_node_for_get_inc)
#define isl_ast_node_for_get_init (*isl_pointers__.p_isl_ast_node_for_get_init)
#define isl_ast_node_for_get_iterator (*isl_pointers__.p_isl_ast_node_for_get_iterator)
#define isl_ast_node_free (*isl_pointers__.p_isl_ast_node_free)
#define isl_ast_node_get_annotation (*isl_pointers__.p_isl_ast_node_get_annotation)
#define isl_ast_node_get_type (*isl_pointers__.p_isl_ast_node_get_type)
#define isl_ast_node_if_get_cond (*isl_pointers__.p_isl_ast_node_if_get_cond)
#define isl_ast_node_if_get_else (*isl_pointers__.p_isl_ast_node_if_get_else)
#define isl_ast_node_if_get_then (*isl_pointers__.p_isl_ast_node_if_get_then)
#define isl_ast_node_list_free (*isl_pointers__.p_isl_ast_node_list_free)
#define isl_ast_node_list_get_ast_node (*isl_pointers__.p_isl_ast_node_list_get_ast_node)
#define isl_ast_node_list_n_ast_node (*isl_pointers__.p_isl_ast_node_list_n_ast_node)
#define isl_ast_node_user_get_expr (*isl_pointers__.p_isl_ast_node_user_get_expr)
#define isl_constraint_set_coefficient_val (*isl_pointers__.p_isl_constraint_set_coefficient_val)
#define isl_constraint_set_constant_val (*isl_pointers__.p_isl_constraint_set_constant_val)
#define isl_id_get_user (*isl_pointers__.p_isl_id_get_user)
#define isl_local_space_get_ctx (*isl_pointers__.p_isl_local_space_get_ctx)
#define isl_map_fix_val (*isl_pointers__.p_isl_map_fix_val)
#define isl_options_set_ast_build_atomic_upper_bound (*isl_pointers__.p_isl_options_set_ast_build_atomic_upper_bound)
#define isl_printer_print_ast_node (*isl_pointers__.p_isl_printer_print_ast_node)
#define isl_printer_print_str (*isl_pointers__.p_isl_printer_print_str)
#define isl_printer_set_output_format (*isl_pointers__.p_isl_printer_set_output_format)
#define isl_pw_aff_mod_val (*isl_pointers__.p_isl_pw_aff_mod_val)
#define isl_schedule_constraints_compute_schedule (*isl_pointers__.p_isl_schedule_constraints_compute_schedule)
#define isl_schedule_constraints_on_domain (*isl_pointers__.p_isl_schedule_constraints_on_domain)
#define isl_schedule_constraints_set_coincidence (*isl_pointers__.p_isl_schedule_constraints_set_coincidence)
#define isl_schedule_constraints_set_proximity (*isl_pointers__.p_isl_schedule_constraints_set_proximity)
#define isl_schedule_constraints_set_validity (*isl_pointers__.p_isl_schedule_constraints_set_validity)
#define isl_set_get_dim_id (*isl_pointers__.p_isl_set_get_dim_id)
#define isl_set_max_val (*isl_pointers__.p_isl_set_max_val)
#define isl_set_min_val (*isl_pointers__.p_isl_set_min_val)
#define isl_set_params (*isl_pointers__.p_isl_set_params)
#define isl_space_align_params (*isl_pointers__.p_isl_space_align_params)
#define isl_space_map_from_domain_and_range (*isl_pointers__.p_isl_space_map_from_domain_and_range)
#define isl_space_set_tuple_name (*isl_pointers__.p_isl_space_set_tuple_name)
#define isl_space_wrap (*isl_pointers__.p_isl_space_wrap)
#define isl_union_map_from_domain_and_range (*isl_pointers__.p_isl_union_map_from_domain_and_range)
#define isl_union_map_range (*isl_pointers__.p_isl_union_map_range)
#define isl_union_set_union (*isl_pointers__.p_isl_union_set_union)
#define isl_union_set_universe (*isl_pointers__.p_isl_union_set_universe)
#define isl_val_2exp (*isl_pointers__.p_isl_val_2exp)
#define isl_val_add_ui (*isl_pointers__.p_isl_val_add_ui)
#define isl_val_copy (*isl_pointers__.p_isl_val_copy)
#define isl_val_free (*isl_pointers__.p_isl_val_free)
#define isl_val_int_from_si (*isl_pointers__.p_isl_val_int_from_si)
#define isl_val_int_from_ui (*isl_pointers__.p_isl_val_int_from_ui)
#define isl_val_mul (*isl_pointers__.p_isl_val_mul)
#define isl_val_neg (*isl_pointers__.p_isl_val_neg)
#define isl_val_sub (*isl_pointers__.p_isl_val_sub)
#define isl_printer_print_union_map (*isl_pointers__.p_isl_printer_print_union_map)
#define isl_pw_aff_get_ctx (*isl_pointers__.p_isl_pw_aff_get_ctx)
#define isl_val_is_int (*isl_pointers__.p_isl_val_is_int)
#define isl_ctx_get_max_operations (*isl_pointers__.p_isl_ctx_get_max_operations)
#define isl_ctx_set_max_operations (*isl_pointers__.p_isl_ctx_set_max_operations)
#define isl_ctx_last_error (*isl_pointers__.p_isl_ctx_last_error)
#define isl_ctx_reset_operations (*isl_pointers__.p_isl_ctx_reset_operations)
#define isl_map_coalesce (*isl_pointers__.p_isl_map_coalesce)
#define isl_printer_print_schedule (*isl_pointers__.p_isl_printer_print_schedule)
#define isl_set_set_dim_id (*isl_pointers__.p_isl_set_set_dim_id)
#define isl_union_map_coalesce (*isl_pointers__.p_isl_union_map_coalesce)
#define isl_multi_val_set_val (*isl_pointers__.p_isl_multi_val_set_val)
#define isl_multi_val_zero (*isl_pointers__.p_isl_multi_val_zero)
#define isl_options_set_schedule_max_coefficient (*isl_pointers__.p_isl_options_set_schedule_max_coefficient)
#define isl_options_set_tile_scale_tile_loops (*isl_pointers__.p_isl_options_set_tile_scale_tile_loops)
#define isl_schedule_copy (*isl_pointers__.p_isl_schedule_copy)
#define isl_schedule_get_map (*isl_pointers__.p_isl_schedule_get_map)
#define isl_schedule_map_schedule_node_bottom_up (*isl_pointers__.p_isl_schedule_map_schedule_node_bottom_up)
#define isl_schedule_node_band_get_permutable (*isl_pointers__.p_isl_schedule_node_band_get_permutable)
#define isl_schedule_node_band_get_space (*isl_pointers__.p_isl_schedule_node_band_get_space)
#define isl_schedule_node_band_tile (*isl_pointers__.p_isl_schedule_node_band_tile)
#define isl_schedule_node_child (*isl_pointers__.p_isl_schedule_node_child)
#define isl_schedule_node_free (*isl_pointers__.p_isl_schedule_node_free)
#define isl_schedule_node_get_child (*isl_pointers__.p_isl_schedule_node_get_child)
#define isl_schedule_node_get_ctx (*isl_pointers__.p_isl_schedule_node_get_ctx)
#define isl_schedule_node_get_type (*isl_pointers__.p_isl_schedule_node_get_type)
#define isl_schedule_node_n_children (*isl_pointers__.p_isl_schedule_node_n_children)
#define isl_union_map_is_equal (*isl_pointers__.p_isl_union_map_is_equal)
#define isl_union_access_info_compute_flow (*isl_pointers__.p_isl_union_access_info_compute_flow)
#define isl_union_access_info_from_sink (*isl_pointers__.p_isl_union_access_info_from_sink)
#define isl_union_access_info_set_may_source (*isl_pointers__.p_isl_union_access_info_set_may_source)
#define isl_union_access_info_set_must_source (*isl_pointers__.p_isl_union_access_info_set_must_source)
#define isl_union_access_info_set_schedule (*isl_pointers__.p_isl_union_access_info_set_schedule)
#define isl_union_flow_free (*isl_pointers__.p_isl_union_flow_free)
#define isl_union_flow_get_may_dependence (*isl_pointers__.p_isl_union_flow_get_may_dependence)
#define isl_union_flow_get_must_dependence (*isl_pointers__.p_isl_union_flow_get_must_dependence)
#define isl_aff_var_on_domain (*isl_pointers__.p_isl_aff_var_on_domain)
#define isl_multi_aff_from_aff (*isl_pointers__.p_isl_multi_aff_from_aff)
#define isl_schedule_get_ctx (*isl_pointers__.p_isl_schedule_get_ctx)
#define isl_multi_aff_set_tuple_id (*isl_pointers__.p_isl_multi_aff_set_tuple_id)
#define isl_multi_aff_dim (*isl_pointers__.p_isl_multi_aff_dim)
#define isl_schedule_get_domain (*isl_pointers__.p_isl_schedule_get_domain)
#define isl_union_set_is_empty (*isl_pointers__.p_isl_union_set_is_empty)
#define isl_union_set_get_space (*isl_pointers__.p_isl_union_set_get_space)
#define isl_union_pw_multi_aff_empty (*isl_pointers__.p_isl_union_pw_multi_aff_empty)
#define isl_union_set_foreach_set (*isl_pointers__.p_isl_union_set_foreach_set)
#define isl_union_set_free (*isl_pointers__.p_isl_union_set_free)
#define isl_multi_union_pw_aff_from_union_pw_multi_aff (*isl_pointers__.p_isl_multi_union_pw_aff_from_union_pw_multi_aff)
#define isl_multi_union_pw_aff_apply_multi_aff (*isl_pointers__.p_isl_multi_union_pw_aff_apply_multi_aff)
#define isl_schedule_insert_partial_schedule (*isl_pointers__.p_isl_schedule_insert_partial_schedule)
#define isl_union_pw_multi_aff_free (*isl_pointers__.p_isl_union_pw_multi_aff_free)
#define isl_pw_multi_aff_project_out_map (*isl_pointers__.p_isl_pw_multi_aff_project_out_map)
#define isl_union_pw_multi_aff_add_pw_multi_aff (*isl_pointers__.p_isl_union_pw_multi_aff_add_pw_multi_aff)
#define isl_schedule_from_domain (*isl_pointers__.p_isl_schedule_from_domain)
#define isl_schedule_sequence (*isl_pointers__.p_isl_schedule_sequence)
#define isl_ast_build_node_from_schedule (*isl_pointers__.p_isl_ast_build_node_from_schedule)
#define isl_ast_node_mark_get_node (*isl_pointers__.p_isl_ast_node_mark_get_node)
#define isl_schedule_node_band_member_get_ast_loop_type (*isl_pointers__.p_isl_schedule_node_band_member_get_ast_loop_type)
#define isl_schedule_node_band_member_set_ast_loop_type (*isl_pointers__.p_isl_schedule_node_band_member_set_ast_loop_type)
#define isl_val_n_abs_num_chunks (*isl_pointers__.p_isl_val_n_abs_num_chunks)
#define isl_val_get_abs_num_chunks (*isl_pointers__.p_isl_val_get_abs_num_chunks)
#define isl_val_int_from_chunks (*isl_pointers__.p_isl_val_int_from_chunks)
#define isl_val_is_neg (*isl_pointers__.p_isl_val_is_neg)
#define isl_version (*isl_pointers__.p_isl_version)
#define isl_options_get_on_error (*isl_pointers__.p_isl_options_get_on_error)
#define isl_ctx_reset_error (*isl_pointers__.p_isl_ctx_reset_error)

typedef struct poly_dr *poly_dr_p;

typedef struct poly_bb *poly_bb_p;

typedef struct scop *scop_p;

typedef unsigned graphite_dim_t;

static inline graphite_dim_t scop_nb_params (scop_p);

/* A data reference can write or read some memory or we
   just know it may write some memory.  */
enum poly_dr_type
{
  PDR_READ,
  /* PDR_MAY_READs are represented using PDR_READS.  This does not
     limit the expressiveness.  */
  PDR_WRITE,
  PDR_MAY_WRITE
};

struct poly_dr
{
  /* An identifier for this PDR.  */
  int id;

  /* The number of data refs identical to this one in the PBB.  */
  int nb_refs;

  /* A pointer to the gimple stmt containing this reference.  */
  gimple *stmt;

  /* A pointer to the PBB that contains this data reference.  */
  poly_bb_p pbb;

  enum poly_dr_type type;

  /* The access polyhedron contains the polyhedral space this data
     reference will access.

     The polyhedron contains these dimensions:

     - The alias set (a):
     Every memory access is classified in at least one alias set.

     - The subscripts (s_0, ..., s_n):
     The memory is accessed using zero or more subscript dimensions.

     - The iteration domain (variables and parameters)

     Do not hardcode the dimensions.  Use the following accessor functions:
     - pdr_alias_set_dim
     - pdr_subscript_dim
     - pdr_iterator_dim
     - pdr_parameter_dim

     Example:

     | int A[1335][123];
     | int *p = malloc ();
     |
     | k = ...
     | for i
     |   {
     |     if (unknown_function ())
     |       p = A;
     |       ... = p[?][?];
     | 	   for j
     |       A[i][j+k] = m;
     |   }

     The data access A[i][j+k] in alias set "5" is described like this:

     | i   j   k   a  s0  s1   1
     | 0   0   0   1   0   0  -5     =  0
     |-1   0   0   0   1   0   0     =  0
     | 0  -1  -1   0   0   1   0     =  0
     | 0   0   0   0   1   0   0     >= 0  # The last four lines describe the
     | 0   0   0   0   0   1   0     >= 0  # array size.
     | 0   0   0   0  -1   0 1335    >= 0
     | 0   0   0   0   0  -1 123     >= 0

     The pointer "*p" in alias set "5" and "7" is described as a union of
     polyhedron:


     | i   k   a  s0   1
     | 0   0   1   0  -5   =  0
     | 0   0   0   1   0   >= 0

     "or"

     | i   k   a  s0   1
     | 0   0   1   0  -7   =  0
     | 0   0   0   1   0   >= 0

     "*p" accesses all of the object allocated with 'malloc'.

     The scalar data access "m" is represented as an array with zero subscript
     dimensions.

     | i   j   k   a   1
     | 0   0   0  -1   15  = 0

     The difference between the graphite internal format for access data and
     the OpenSop format is in the order of columns.
     Instead of having:

     | i   j   k   a  s0  s1   1
     | 0   0   0   1   0   0  -5     =  0
     |-1   0   0   0   1   0   0     =  0
     | 0  -1  -1   0   0   1   0     =  0
     | 0   0   0   0   1   0   0     >= 0  # The last four lines describe the
     | 0   0   0   0   0   1   0     >= 0  # array size.
     | 0   0   0   0  -1   0 1335    >= 0
     | 0   0   0   0   0  -1 123     >= 0

     In OpenScop we have:

     | a  s0  s1   i   j   k   1
     | 1   0   0   0   0   0  -5     =  0
     | 0   1   0  -1   0   0   0     =  0
     | 0   0   1   0  -1  -1   0     =  0
     | 0   1   0   0   0   0   0     >= 0  # The last four lines describe the
     | 0   0   1   0   0   0   0     >= 0  # array size.
     | 0  -1   0   0   0   0 1335    >= 0
     | 0   0  -1   0   0   0 123     >= 0

     The OpenScop access function is printed as follows:

     | 1  # The number of disjunct components in a union of access functions.
     | R C O I L P  # Described bellow.
     | a  s0  s1   i   j   k   1
     | 1   0   0   0   0   0  -5     =  0
     | 0   1   0  -1   0   0   0     =  0
     | 0   0   1   0  -1  -1   0     =  0
     | 0   1   0   0   0   0   0     >= 0  # The last four lines describe the
     | 0   0   1   0   0   0   0     >= 0  # array size.
     | 0  -1   0   0   0   0 1335    >= 0
     | 0   0  -1   0   0   0 123     >= 0

     Where:
     - R: Number of rows.
     - C: Number of columns.
     - O: Number of output dimensions = alias set + number of subscripts.
     - I: Number of input dimensions (iterators).
     - L: Number of local (existentially quantified) dimensions.
     - P: Number of parameters.

     In the example, the vector "R C O I L P" is "7 7 3 2 0 1".  */
  isl_map *accesses;
  isl_set *subscript_sizes;
};

#define PDR_ID(PDR) (PDR->id)
#define PDR_NB_REFS(PDR) (PDR->nb_refs)
#define PDR_PBB(PDR) (PDR->pbb)
#define PDR_TYPE(PDR) (PDR->type)
#define PDR_ACCESSES(PDR) (NULL)

void new_poly_dr (poly_bb_p, gimple *, enum poly_dr_type,
		  isl_map *, isl_set *);
void debug_pdr (poly_dr_p);
void print_pdr (FILE *, poly_dr_p);

static inline bool
pdr_read_p (poly_dr_p pdr)
{
  return PDR_TYPE (pdr) == PDR_READ;
}

/* Returns true when PDR is a "write".  */

static inline bool
pdr_write_p (poly_dr_p pdr)
{
  return PDR_TYPE (pdr) == PDR_WRITE;
}

/* Returns true when PDR is a "may write".  */

static inline bool
pdr_may_write_p (poly_dr_p pdr)
{
  return PDR_TYPE (pdr) == PDR_MAY_WRITE;
}

/* POLY_BB represents a blackbox in the polyhedral model.  */

struct poly_bb
{
  /* Pointer to a basic block or a statement in the compiler.  */
  gimple_poly_bb_p black_box;

  /* Pointer to the SCOP containing this PBB.  */
  scop_p scop;

  /* The iteration domain of this bb.  The layout of this polyhedron
     is I|G with I the iteration domain, G the context parameters.

     Example:

     for (i = a - 7*b + 8; i <= 3*a + 13*b + 20; i++)
       for (j = 2; j <= 2*i + 5; j++)
         for (k = 0; k <= 5; k++)
           S (i,j,k)

     Loop iterators: i, j, k
     Parameters: a, b

     | i >=  a -  7b +  8
     | i <= 3a + 13b + 20
     | j >= 2
     | j <= 2i + 5
     | k >= 0
     | k <= 5

     The number of variables in the DOMAIN may change and is not
     related to the number of loops in the original code.  */
  isl_set *domain;
  isl_set *iterators;

  /* The data references we access.  */
  vec<poly_dr_p> drs;

  /* The last basic block generated for this pbb.  */
  basic_block new_bb;
};

#define PBB_BLACK_BOX(PBB) ((gimple_poly_bb_p) PBB->black_box)
#define PBB_SCOP(PBB) (PBB->scop)
#define PBB_DRS(PBB) (PBB->drs)

extern poly_bb_p new_poly_bb (scop_p, gimple_poly_bb_p);
extern void print_pbb_domain (FILE *, poly_bb_p);
extern void print_pbb (FILE *, poly_bb_p);
extern void print_scop_context (FILE *, scop_p);
extern void print_scop (FILE *, scop_p);
extern void debug_pbb_domain (poly_bb_p);
extern void debug_pbb (poly_bb_p);
extern void print_pdrs (FILE *, poly_bb_p);
extern void debug_pdrs (poly_bb_p);
extern void debug_scop_context (scop_p);
extern void debug_scop (scop_p);
extern void print_scop_params (FILE *, scop_p);
extern void debug_scop_params (scop_p);
extern void print_iteration_domain (FILE *, poly_bb_p);
extern void print_iteration_domains (FILE *, scop_p);
extern void debug_iteration_domain (poly_bb_p);
extern void debug_iteration_domains (scop_p);
extern void print_isl_set (FILE *, isl_set *);
extern void print_isl_map (FILE *, isl_map *);
extern void print_isl_union_map (FILE *, isl_union_map *);
extern void print_isl_aff (FILE *, isl_aff *);
extern void print_isl_constraint (FILE *, isl_constraint *);
extern void print_isl_schedule (FILE *, isl_schedule *);
extern void debug_isl_schedule (isl_schedule *);
extern void print_isl_ast (FILE *, isl_ast_node *);
extern void debug_isl_ast (isl_ast_node *);
extern void debug_isl_set (isl_set *);
extern void debug_isl_map (isl_map *);
extern void debug_isl_union_map (isl_union_map *);
extern void debug_isl_aff (isl_aff *);
extern void debug_isl_constraint (isl_constraint *);
extern void debug_gmp_value (mpz_t);
extern void debug_scop_pbb (scop_p scop, int i);
extern void print_schedule_ast (FILE *, __isl_keep isl_schedule *, scop_p);
extern void debug_schedule_ast (__isl_keep isl_schedule *, scop_p);

/* The basic block of the PBB.  */

static inline basic_block
pbb_bb (poly_bb_p pbb)
{
  return GBB_BB (PBB_BLACK_BOX (pbb));
}

static inline int
pbb_index (poly_bb_p pbb)
{
  return pbb_bb (pbb)->index;
}

/* The loop of the PBB.  */

static inline loop_p
pbb_loop (poly_bb_p pbb)
{
  return gbb_loop (PBB_BLACK_BOX (pbb));
}

/* The scop that contains the PDR.  */

static inline scop_p
pdr_scop (poly_dr_p pdr)
{
  return PBB_SCOP (PDR_PBB (pdr));
}

/* Set black box of PBB to BLACKBOX.  */

static inline void
pbb_set_black_box (poly_bb_p pbb, gimple_poly_bb_p black_box)
{
  pbb->black_box = black_box;
}

/* A helper structure to keep track of data references, polyhedral BBs, and
   alias sets.  */

struct dr_info
{
  enum {
    invalid_alias_set = -1
  };
  /* The data reference.  */
  data_reference_p dr;

  /* The polyhedral BB containing this DR.  */
  poly_bb_p pbb;

  /* ALIAS_SET is the SCC number assigned by a graph_dfs of the alias graph.
     -1 is an invalid alias set.  */
  int alias_set;

  /* Construct a DR_INFO from a data reference DR, an ALIAS_SET, and a PBB.  */
  dr_info (data_reference_p dr, poly_bb_p pbb,
	   int alias_set = invalid_alias_set)
    : dr (dr), pbb (pbb), alias_set (alias_set) {}
};

/* A SCOP is a Static Control Part of the program, simple enough to be
   represented in polyhedral form.  */
struct scop
{
  /* A SCOP is defined as a SESE region.  */
  sese_info_p scop_info;

  /* Number of parameters in SCoP.  */
  graphite_dim_t nb_params;

  /* The maximum alias set as assigned to drs by build_alias_sets.  */
  unsigned max_alias_set;

  /* All the basic blocks in this scop that contain memory references
     and that will be represented as statements in the polyhedral
     representation.  */
  vec<poly_bb_p> pbbs;

  /* All the data references in this scop.  */
  vec<dr_info> drs;

  /* The context describes known restrictions concerning the parameters
     and relations in between the parameters.

  void f (int8_t a, uint_16_t b) {
    c = 2 a + b;
    ...
  }

  Here we can add these restrictions to the context:

  -128 >= a >= 127
     0 >= b >= 65,535
     c = 2a + b  */
  isl_set *param_context;

  /* The context used internally by isl.  */
  isl_ctx *isl_context;

  /* SCoP original schedule.  */
  isl_schedule *original_schedule;

  /* SCoP transformed schedule.  */
  isl_schedule *transformed_schedule;

  /* The data dependence relation among the data references in this scop.  */
  isl_union_map *dependence;
};

extern scop_p new_scop (edge, edge);
extern void free_scop (scop_p);
extern gimple_poly_bb_p new_gimple_poly_bb (basic_block, vec<data_reference_p>,
					    vec<scalar_use>, vec<tree>);
extern bool apply_poly_transforms (scop_p);

/* Set the region of SCOP to REGION.  */

static inline void
scop_set_region (scop_p scop, sese_info_p region)
{
  scop->scop_info = region;
}

/* Returns the number of parameters for SCOP.  */

static inline graphite_dim_t
scop_nb_params (scop_p scop)
{
  return scop->nb_params;
}

/* Set the number of params of SCOP to NB_PARAMS.  */

static inline void
scop_set_nb_params (scop_p scop, graphite_dim_t nb_params)
{
  scop->nb_params = nb_params;
}

extern void scop_get_dependences (scop_p scop);

bool
carries_deps (__isl_keep isl_union_map *schedule,
	      __isl_keep isl_union_map *deps,
	      int depth);

extern bool build_poly_scop (scop_p);
extern bool graphite_regenerate_ast_isl (scop_p);
extern void build_scops (vec<scop_p> *);
extern void dot_all_sese (FILE *, vec<sese_l> &);
extern void dot_sese (sese_l &);
extern void dot_cfg ();
extern const char *get_isl_version (bool);

#endif
