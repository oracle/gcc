#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "hash-set.h"
#include "machmode.h"
#include "vec.h"
#include "double-int.h"
#include "input.h"
#include "alias.h"
#include "symtab.h"
#include "options.h"
#include "wide-int.h"
#include "inchash.h"
#include "tree.h"
#include "fold-const.h"
#include "hashtab.h"
#include "tm.h"
#include "hard-reg-set.h"
#include "function.h"
#include "rtl.h"
#include "flags.h"
#include "statistics.h"
#include "real.h"
#include "fixed-value.h"
#include "insn-config.h"
#include "profile-count.h"
#include "expmed.h"
#include "dojump.h"
#include "explow.h"
#include "calls.h"
#include "varasm.h"
#include "stmt.h"
#include "expr.h"
#include "gimple-pretty-print.h"
#include "predict.h"
#include "dominance.h"
#include "cfg.h"
#include "cfgloop.h"
#include "cfghooks.h"
#include "basic-block.h"
#include "tree-ssa-alias.h"
#include "internal-fn.h"
#include "gimple-expr.h"
#include "is-a.h"
#include "gimple.h"
#include "gimplify.h"
#include "gimple-iterator.h"
#include "tree-cfg.h"
#include "gimplify-me.h"
#include "gimple-ssa.h"
#include "tree-ssa-operands.h"
#include "tree-phinodes.h"
#include "ssa-iterators.h"
#include "dumpfile.h"
#include "tree-pass.h"
#include "ipa-ref.h"
#include "plugin-api.h"
#include "cgraph.h"
#include "function.h"

#include "stringpool.h"
#include "tree-vrp.h"
#include "tree-ssanames.h"
#include "tree-into-ssa.h"

typedef vec <tree> expr_vec;

// FIXME Copied from tree-ssa-math-opts.c
static void
build_and_insert_cast (gimple_stmt_iterator *gsi, location_t loc,
                       tree type, tree val)
{
  gimple *use_stmt;
  imm_use_iterator use_iter;
  use_operand_p use_p;

  tree result = make_ssa_name (type, NULL);

  gimple *stmt = gimple_build_assign (result, NOP_EXPR, val);
  gimple_set_location (stmt, loc);

  FOR_EACH_IMM_USE_STMT (use_stmt, use_iter, val)
    {
      FOR_EACH_IMM_USE_ON_STMT (use_p, use_iter)
        SET_USE (use_p, result);
      update_stmt (use_stmt);
    }

  gsi_insert_before (gsi, stmt, GSI_SAME_STMT);
  update_stmt (stmt);
}

static void insert_casts (basic_block body, int n)
{
  gimple_stmt_iterator iter;
  gimple *body_last_stmt;
  tree cond_rhs;

  // FIXME Hardcoded loop id.
  if (n == 2)
    {
      iter = gsi_last_bb (body);
      body_last_stmt = gsi_stmt (iter);
      cond_rhs = gimple_cond_rhs (body_last_stmt);
      build_and_insert_cast (&iter, UNKNOWN_LOCATION,
                             short_integer_type_node, cond_rhs);
    }

}

static unsigned int
num_phis (basic_block bb)
{
  gimple_stmt_iterator si;
  unsigned int np = 0;
  for (si = gsi_start_phis (bb); !gsi_end_p (si); gsi_next (&si))
    np++;
  return np;
}

// FIXME This pass is in a bad state.
//        * Get rid of warnings.
//        * Refactor to separate functions.
//        * Minimize number of update_stmt.
//        * Find where you can reduce duplicate work.
//        * Think of consistent variable namings.
static unsigned int
list_find_pipeline (void)
{
  struct loop * loop;
  edge e;
  edge_iterator ei;
  tree op0, op1, op01;
  tree def_rhs, cond_rhs;
  tree phi_op;
  tree back_edge_phi_op;

  tree constant_null_pointer;

  gimple *use_stmt;
  imm_use_iterator use_iter;
  use_operand_p use_p;

  gimple *new_next_stmt;
  // FIXME this is an awful name.
  tree new_new_list;

  tree new_list;
  tree old_list;
  tree copy_lhs;
  tree phi_result;
  gimple *next_stmt;
  gimple *next_copy;
  gimple *phi_stmt;
  gimple_stmt_iterator cond_iterator;
  gimple *last_stmt;
  gimple *exit_phi_stmt;
  gimple *exit_first_stmt;

  tree exit_lhs;
  tree exit_rhs;

  gimple *def_stmt;

  basic_block use_bb;

  edge entry_edge;
  edge exit_edge = NULL;
  edge back_edge;
  edge body_entry_edge = NULL;
  edge body_exit_edge = NULL;
  edge body_latch_edge = NULL;

  edge new_exit_edge;
  edge new_body_entry_edge = NULL;
  edge new_body_exit_edge;
  edge new_body_latch_edge = NULL;

  basic_block body;
  basic_block latch;

  basic_block new_header;
  basic_block new_body;

  gimple_stmt_iterator iter;

  bool complex_case;

  FILE * dump_f = dump_file;

  loop_optimizer_init (LOOPS_HAVE_SIMPLE_LATCHES | LOOPS_HAVE_PREHEADERS);
  record_loop_exits ();
  initialize_original_copy_tables ();

  FOR_EACH_LOOP (loop, LI_ONLY_INNERMOST)
    {
      if (loop->header->preds->length () != 2)
        continue;

      if (loop->header->succs->length () != 2)
        continue;

      // This will we overwritten so we keep it.
      latch = loop->latch;
      FOR_EACH_EDGE (e, ei, loop->header->preds)
        {
          if (e->src != latch)
            entry_edge = e;
          else
            back_edge = e;
        }

      FOR_EACH_EDGE (e, ei, loop->header->succs)
        {
          if (loop_exit_edge_p (loop, e))
            exit_edge = e;
          else
            body_entry_edge = e;
        }

      // The predecessor must dominate the header.
      if (entry_edge->src->succs->length () != 1)
        continue;

      // We only want a loop of header, body and latch.
      if (loop->num_nodes != 3)
        continue;

      // We only want exactly one phi node.
      if (num_phis (loop->header) != 1)
        continue;

      // This is the basic block in the loop,
      // where most of the work is done.
      body = body_entry_edge->dest;
      // Body must have exactly two successors.
      if (body->succs->length () != 2)
        continue;

      if (!can_duplicate_block_p (loop->header))
        continue;
      if (!can_duplicate_block_p (body))
        continue;

      FOR_EACH_EDGE (e, ei, body->succs)
        {
          if (e->dest == latch)
            body_latch_edge = e;
          else
            body_exit_edge = e;
        }

      // Both exit edges must point to the same bb.
      if (body_exit_edge->dest != exit_edge->dest)
        continue;

      // Find and check the condition.
      cond_iterator = gsi_last_bb (loop->header);
      last_stmt = gsi_stmt (cond_iterator);
      if (dump_f)
        print_gimple_stmt (dump_f, last_stmt, 0, 0);

      if (gimple_code (last_stmt) != GIMPLE_COND)
        continue;

      if (gimple_cond_code (last_stmt) != NE_EXPR)
        continue;

      cond_rhs = gimple_cond_rhs (last_stmt);
      if (!operand_equal_p (cond_rhs, null_pointer_node, 0))
        continue;

      new_list = gimple_cond_lhs (last_stmt);

      // Also the exit node must have only one phi node.
      // FIXME This is not a sufficiently safe check.
      iter = gsi_start_bb (exit_edge->dest);
      exit_first_stmt = gsi_stmt (iter);
      exit_lhs = gimple_assign_lhs (exit_first_stmt);
      exit_rhs = gimple_assign_rhs1 (exit_first_stmt);

      if (exit_rhs == new_list)
        {
          if (exit_edge->dest->preds->length () != 2)
            continue;

          // FIXME Do all alteration *after* the analysis phase.
          gsi_remove (&iter, true);
          exit_phi_stmt = create_phi_node (exit_lhs, exit_edge->dest);

          FOR_EACH_EDGE (e, ei, exit_edge->dest->preds)
            {
              SET_PHI_ARG_DEF (exit_phi_stmt, e->dest_idx, exit_rhs);
            }

          if (dump_f)
            fprintf (dump_f, "list-find-pipeline: replace assignment by phi node in loop number %d\n", loop->num);

        }
      else if (num_phis (exit_edge->dest) != 1)
        {
          continue;
        }

      // Find the defining statement.
      def_stmt = SSA_NAME_DEF_STMT (new_list);
      if (dump_f)
        print_gimple_stmt (dump_f, def_stmt, 0, 0);

      if (gimple_code (def_stmt) == GIMPLE_ASSIGN)
        {
          // Is it the first statement in the header?
          if (def_stmt != gsi_stmt (gsi_start_bb (loop->header)))
            continue;
          complex_case = true;
          next_stmt = def_stmt;
        }
      else if (gimple_code (def_stmt) == GIMPLE_PHI)
        {
          // Is it the last statement in the latch?
          back_edge_phi_op = PHI_ARG_DEF_FROM_EDGE (def_stmt, back_edge);
          next_stmt = SSA_NAME_DEF_STMT (back_edge_phi_op);
          if (next_stmt != gsi_stmt (gsi_last_bb (latch)))
            continue;
          complex_case = false;
          new_list = gimple_assign_lhs (next_stmt);
        }
      else
        continue;

      next_copy = gimple_copy (next_stmt);

      // Analyze the defining statement (acutally its copy).
      // It should be of the form new_list = old_list->next.
      def_rhs = gimple_assign_rhs1 (next_copy);
      if (TREE_CODE (def_rhs) != COMPONENT_REF)
        continue;
      if (TREE_OPERAND_LENGTH (def_rhs) != 3)
        continue;

      op1 = TREE_OPERAND (def_rhs, 1);
      if (TREE_CODE (op1) != FIELD_DECL)
        continue;
      if (TREE_OPERAND_LENGTH (op1) != 0)
        continue;

      op0 = TREE_OPERAND (def_rhs, 0);
      if (TREE_CODE (op0) != MEM_REF)
        continue;
      if (TREE_OPERAND_LENGTH (op0) != 2)
        continue;

      op01 = TREE_OPERAND (op0, 1);
      if (TREE_CODE (op01) != INTEGER_CST)
        continue;
      if (!operand_equal_p (op01, integer_zero_node, 0))
        continue;

      old_list = TREE_OPERAND (op0, 0);
      if (TREE_CODE (old_list) != SSA_NAME)
        continue;

      if (TREE_OPERAND_LENGTH (old_list) != 0)
        continue;

      // The old list must be defined by the phi node.
      phi_stmt = SSA_NAME_DEF_STMT (old_list);
      if (gimple_code (phi_stmt) != GIMPLE_PHI)
        continue;

      if (gimple_phi_num_args (phi_stmt) != 2)
        continue;

      if (dump_f)
        print_gimple_stmt (dump_f, phi_stmt, 0, 0);

      if (!complex_case)
        gcc_assert (phi_stmt == def_stmt);

      // The operand coming in by the entry edge.
      phi_op = PHI_ARG_DEF_FROM_EDGE (phi_stmt, entry_edge);
      if (TREE_CODE (phi_op) != SSA_NAME)
        continue;

      // We found a feasible loop.
      if (dump_f)
        fprintf (dump_f, "list-find-pipeline: Found feasible loop number %d\n", loop->num);

      insert_casts (body, loop->num);

      phi_result = gimple_phi_result (phi_stmt);

      if (complex_case)
        {
          // Set the new pointer base in next_copy.
          TREE_OPERAND (op0, 0) = phi_op;

          copy_lhs = copy_ssa_name (new_list, next_copy);
          gimple_assign_set_lhs (next_copy, copy_lhs);
          update_stmt (next_copy);

          // The next_copy must be at the end of the single predecessor block.
          iter = gsi_last_bb (entry_edge->src);
          gsi_insert_after (&iter, next_copy, GSI_NEW_STMT);

          SET_PHI_ARG_DEF (phi_stmt, entry_edge->dest_idx, copy_lhs);

          // Remove the original next_stmt temporarily, insert it later.
          iter = gsi_for_stmt (next_stmt);
          gsi_remove (&iter, false);

          // Set all uses of new_list in header and body to the
          // name we get via the phi node.
          // Also in the phi node of the block after the loop.
          FOR_EACH_IMM_USE_STMT (use_stmt, use_iter, new_list)
            {
              use_bb = gimple_bb (use_stmt);

              if (use_bb == loop->header ||
                  use_bb == body ||
                  use_bb == exit_edge->dest)
                {
                  FOR_EACH_IMM_USE_ON_STMT (use_p, use_iter)
                    SET_USE (use_p, phi_result);
                  update_stmt (use_stmt);
                }
            }

          // Insert the next_stmt at the top of body.
          iter = gsi_start_bb (body);
          gsi_insert_before (&iter, next_stmt, GSI_NEW_STMT);
          // Correct the phi argument.
          SET_PHI_ARG_DEF (phi_stmt, back_edge->dest_idx, new_list);

          if (dump_f)
            print_gimple_stmt (dump_f, next_copy, 0, 0);
        }
      else
        {
          // Remove the original next_stmt temporarily, insert it later.
          iter = gsi_for_stmt (next_stmt);
          gsi_remove (&iter, false);
          iter = gsi_start_bb (body);
          gsi_insert_before (&iter, next_stmt, GSI_NEW_STMT);
        }

      // Unroll once.
      new_header = duplicate_block (loop->header, body_latch_edge, body);
      FOR_EACH_EDGE (e, ei, new_header->succs)
        {
          if (e->dest != exit_edge->dest)
            new_body_entry_edge = e;
          else
            new_exit_edge = e;
        }
      new_body = duplicate_block (body, new_body_entry_edge, new_header);
      FOR_EACH_EDGE (e, ei, new_body->succs)
        {
          if (e->dest != exit_edge->dest)
            new_body_latch_edge = e;
          else
            new_body_exit_edge = e;
        }

      redirect_edge_succ (new_body_latch_edge, latch);

      set_immediate_dominator (CDI_DOMINATORS, new_header, body);
      set_immediate_dominator (CDI_DOMINATORS, new_body, new_header);
      set_immediate_dominator (CDI_DOMINATORS, latch, new_body);

      remove_phi_nodes (new_header);

      iter = gsi_start_phis (exit_edge->dest);
      exit_phi_stmt = gsi_stmt (iter);
      constant_null_pointer = null_pointer_node;

      SET_PHI_ARG_DEF (exit_phi_stmt, new_exit_edge->dest_idx, constant_null_pointer);
      SET_PHI_ARG_DEF (exit_phi_stmt, new_body_exit_edge->dest_idx, new_list);

      update_ssa (TODO_update_ssa);

      SET_PHI_ARG_DEF (exit_phi_stmt, new_body_exit_edge->dest_idx, new_list);

      // We know this is the new list, since we put it there.
      iter = gsi_start_bb (new_body);
      new_next_stmt = gsi_stmt (iter);
      new_new_list = gimple_assign_lhs (new_next_stmt);
      SET_PHI_ARG_DEF (phi_stmt, back_edge->dest_idx, new_new_list);
      update_stmt (phi_stmt);
      update_stmt (new_next_stmt);

      FOR_EACH_IMM_USE_STMT (use_stmt, use_iter, phi_result)
        {
          use_bb = gimple_bb (use_stmt);

          if (use_bb == new_header ||
              use_bb == new_body)
            {
              FOR_EACH_IMM_USE_ON_STMT (use_p, use_iter)
                {
                  SET_USE (use_p, new_list);
                }
              update_stmt (use_stmt);
            }
        }
    }

  free_original_copy_tables ();
  release_recorded_exits (cfun);


  return 0;
}

static bool
gate_list_find_pipeline (void)
{
  return list_find_pipe;
}

namespace {

const pass_data pass_data_list_find_pipeline =
{
  GIMPLE_PASS, /* type */
  "list-find-pipeline", /* name */
  OPTGROUP_NONE, /* optinfo_flags */
  TV_NONE, /* tv_id */
  PROP_gimple_any, /* properties_required */
  0, /* properties_provided */
  0, /* properties_destroyed */
  0, /* todo_flags_start */
  0, /* todo_flags_finish */
};

class pass_list_find_pipeline: public gimple_opt_pass
{
public:
  pass_list_find_pipeline (gcc::context *ctxt)
    : gimple_opt_pass (pass_data_list_find_pipeline, ctxt)
  {}

  /* opt_pass methods: */
  bool gate (function *) { return gate_list_find_pipeline (); }
  unsigned int execute (function *) { return list_find_pipeline (); }

};

} // anon namespace

gimple_opt_pass *
make_pass_list_find_pipeline (gcc::context *ctxt)
{
  return new pass_list_find_pipeline (ctxt);
}
