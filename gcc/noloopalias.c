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
#include "expmed.h"
#include "predict.h"
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
#include "tree-ssa.h"
#include "tree-into-ssa.h"

typedef vec<tree> expr_vec;

static const char *uninlined_function_postfix = "loop";

/* If a callback returns a negative number,
 * then we immediately bail out and return the
 * return value from the callback.  */
typedef int (*stmt_callback) (gimple *stmt, void *g);
typedef int (*op_callback) (gimple *stmt, tree op, unsigned index, void *g);


/* Iterator helper, which can be used to iterate over all basic blocks,
 * their statements and the statement's operands in a loop.
 * For each statement and operator a callback will be invoked.
 * If any callback invocation returns a non-null return code,
 * the iteration will be stopped immediately and the non-null
 * return code will be returned.  */

static int
for_each_operand_in_loop (const struct loop *loop, stmt_callback stmt_cb, op_callback op_cb, void *g)
{
  basic_block *bbs;
  unsigned num_nodes;
  unsigned i;
  int ret = 0;

  /* Get the basic blocks.  */
  bbs = get_loop_body (loop);
  num_nodes = loop->num_nodes;

  /* Iterate over basic blocks. */
  for (i = 0; i < num_nodes; i++)
    {
      basic_block bb;
      bb = bbs[i];

      /* Iterate over statements in bb.  */
      gimple_stmt_iterator iter = gsi_start_bb (bb);
      for ( ; !gsi_end_p (iter); gsi_next (&iter))
	{
	  gimple *stmt;
	  unsigned j;
	  stmt = gsi_stmt (iter);

	  if (stmt_cb)
	    {
	      ret = stmt_cb(stmt, g);
	      if (ret)
		goto end;
	    }

	  /* Iterate over operands in stmt.  */
	  for (j = 0; j < gimple_num_ops (stmt); j++)
	    {
	      tree op;
	      op = gimple_op (stmt, j);
	      if (op)
		{
		  if (op_cb)
		    {
		      ret = op_cb (stmt, op, j, g);
		      if (ret)
			goto end;
		    }
		}
	    }
	}
    }

end:
  /* Free the bb list.  */
  free (bbs);

  return ret;
}

/* Given stmt is an assign statement and given op is its LHS.  */

static bool
is_defined_in_stmt (const gimple *stmt, const tree op)
{
  if (gimple_code (stmt) == GIMPLE_ASSIGN)
    {
      tree lhs = gimple_assign_lhs (stmt);
      if (lhs == op)
        return true;
    }
  return false;
}

/* Check if op is LHS of assignment within current bb.  */

static bool
is_defined_inside (gimple *stmt, const tree op)
{
  /* Get a gsi for the bb stmt belongs to (pointing to stmt).  */
  gimple_stmt_iterator gsi = gsi_for_stmt (stmt);

  /* Start search in previous stmt.  */
  gsi_prev (&gsi);

  /* Iterate over all previous stmts in bb.  */
  for (; !gsi_end_p (gsi); gsi_prev (&gsi))
    {
      if (is_defined_in_stmt (gsi_stmt (gsi), op))
        return true;
    }
  return false;
}

static int
is_gimple_call (gimple *stmt, void *g)
{
  (void)g;
  return (gimple_code (stmt) == GIMPLE_CALL);
}

/* Add operand to vector if:
    - it is not LHS of assignment, and
    - op is VAR_DECL or PARM_DECL, and
    - op is not yet in v, and
    - op is not defined inside of stmt's bb.  */

static int
collect_cb (gimple *stmt, tree op, unsigned index, void *g)
{
  expr_vec *v = (expr_vec *) g;
  enum tree_code code;

  /* Whitelist LHS of assignments.  */
  if (gimple_code (stmt) == GIMPLE_ASSIGN && index == 0)
    return 0;

  code = TREE_CODE (op);

  /* Whitelist of supported codes */
  if (code == INTEGER_CST ||
      code == SSA_NAME ||
      code == MEM_REF)
    return 0;

  /* Filter out op, which are not VAR_DECL or PARM_DECL.  */
  if (code != VAR_DECL && code != PARM_DECL)
    return -1;

  /* Filter out op without DECL_NAME. */
  if (!DECL_NAME (op))
    return -1;

  /* Filter out if op already included.  */
  if (v->contains (op))
    return 0;

  /* Filter out ops, which are defined inside bb. */
  if (is_defined_inside (stmt, op))
    return 0;

  if (dump_file)
    {
    fprintf (dump_file, "Found new element for arguments list: ");
      const char *name = "<unknown name>";
      if (DECL_NAME(op) && IDENTIFIER_POINTER (DECL_NAME (op)))
	name = identifier_to_locale (IDENTIFIER_POINTER (DECL_NAME (op)));
      fprintf (dump_file, "'%s'\n", name);
    }

  v->safe_push(op);
  return 0;
}

/* Return unique list of operands in the given loop,
   which are defined outside of the loop.  */

static expr_vec
collect_def_outside (struct loop *loop)
{
  expr_vec v = vNULL;
  int ret = for_each_operand_in_loop (loop, NULL, collect_cb, (void *) &v);
  if (ret)
    return vNULL;

  return v;
}

/* Return non-null if loop contains a call statement with operands.  */

static int
contains_call (struct loop *loop)
{
  return for_each_operand_in_loop (loop, is_gimple_call, NULL, NULL);
}

struct var_to_parm_cb_p
{
  tree d;
  tree d_parm;
};

/* Replace op in stmt by p->d_parm if op == p->d.  */

static int
var_to_parm_cb (gimple *stmt, tree op, unsigned index, void *g)
{
  struct var_to_parm_cb_p *p = (struct var_to_parm_cb_p *) g;
  if (p->d == op)
    {
      gimple_set_op (stmt, index, p->d_parm);
    }
  return 0;
}

/* Replace ops in all stmts of loop by p->d_parm if op == p->d.  */

static void
convert_var_to_parm_uses (struct loop *loop, tree d, tree d_parm)
{
  struct var_to_parm_cb_p c;
  c.d = d;
  c.d_parm = d_parm;
  for_each_operand_in_loop (loop, NULL, var_to_parm_cb, (void *) &c);
}

/* Create a function declaration with void return type.  */

static tree
build_void_function_declaration (struct loop *loop, tree name, expr_vec parameter_vector, vec<tree> *args)
{
  /* We use NULL_TREE as placeholder here.
     This will be fixed later on.  */
  tree type = NULL_TREE;

  /* Create the function declaration.  */
  tree decl = build_decl (input_location, FUNCTION_DECL, name, type);

  TREE_STATIC (decl) = 1;
  TREE_USED (decl) = 1;
  DECL_ARTIFICIAL (decl) = 1;
  DECL_NAMELESS (decl) = 0;
  DECL_IGNORED_P (decl) = 0;
  TREE_PUBLIC (decl) = 0;
  DECL_UNINLINABLE (decl) = 1;
  DECL_EXTERNAL (decl) = 0;
  DECL_CONTEXT (decl) = NULL_TREE;
  DECL_INITIAL (decl) = make_node (BLOCK);
  BLOCK_SUPERCONTEXT (DECL_INITIAL (decl)) = decl;
  DECL_ATTRIBUTES (decl) = DECL_ATTRIBUTES (cfun->decl);

  /* Create result declaration.  */
  tree t = build_decl (DECL_SOURCE_LOCATION (decl),
		  RESULT_DECL, NULL_TREE, void_type_node);

  DECL_ARTIFICIAL (t) = 1;
  DECL_IGNORED_P (t) = 1;
  DECL_CONTEXT (t) = decl;
  DECL_RESULT (decl) = t;

  /* Now we prepare several collections from the parameter vector.  */

  tree t_first = NULL; /* First element of decl chain.  */
  tree t_last = NULL; /* Last element of decl chain.  */
  tree types = NULL_TREE; /* TREE_LIST of types.  */
  tree *pp = &types; /* Helper to create the TREE_LIST. */

  /* Prepare the arguments collected earlier on.  */
  int i = -1;
  tree d = NULL;
  FOR_EACH_VEC_ELT (parameter_vector, i, d)
    {
      /* Append parameter to args vector.  */
      args->safe_push (d);

      /* Get parameter type.  */
      tree type_new = copy_node (TREE_TYPE (d));

      /* Append type to tree list.  */
      *pp = build_tree_list (NULL, type_new);
      pp = &TREE_CHAIN (*pp);

      /* Allocate and initialize parameter declaration.  */
      tree d_parm = build_decl (DECL_SOURCE_LOCATION (decl), PARM_DECL,
                           DECL_NAME (d), type_new);
      DECL_ARG_TYPE (d_parm) = TREE_TYPE (d);
      DECL_ARTIFICIAL (d_parm) = 1;
      DECL_CONTEXT (d_parm) = decl;
      if (POINTER_TYPE_P (TREE_TYPE (d)) && TREE_CODE (d) == VAR_DECL)
	TYPE_RESTRICT (TREE_TYPE (d_parm)) = 1;

      /* Convert the loop to use the parameters.  */
      convert_var_to_parm_uses (loop, d, d_parm);

      if (!t_first)
        {
          t_first = d_parm;
          t_last = d_parm;
        }
      else
        {
          DECL_CHAIN (t_last) = d_parm;
          t_last = d_parm;
        }
    }

  /* Replace the placeholder type by the actual type (void f(args...)).  */
  type = build_function_type (void_type_node, types);
  TREE_TYPE (decl) = type;
  DECL_ARGUMENTS (decl) = t_first;

  /* Allocate memory for the function structure.  The call to
     allocate_struct_function clobbers CFUN, so we need to restore
     it afterward.  */
  push_struct_function (decl);
  cfun->function_end_locus = DECL_SOURCE_LOCATION (decl);
  init_tree_ssa (cfun);
  pop_cfun ();

  return decl;
}

/* Converts the loop body into its own function
 * with the given arguments.
 *
 * This is inspired by create_omp_child_function and create_loop_fn.  */

static void
outline_loop (struct loop *loop, expr_vec parameter_vector)
{
  /* Build function declaration.  */
  char postfix[strlen(uninlined_function_postfix)+12];
  snprintf (postfix, sizeof (postfix), "%s%i", uninlined_function_postfix, loop->num);
  tree name = clone_function_name (cfun->decl, postfix);
  vec<tree> args = vNULL; /* Vector of parameters. */
  tree decl = build_void_function_declaration (loop, name, parameter_vector, &args);

  /* Fill the function body.  */
  gimple_seq body = NULL;
  gimple *bind = gimple_build_bind (NULL, body, DECL_INITIAL (decl));
  gimple_seq_add_stmt (&body, bind);

  /* We'll create a CFG for child_fn, so no gimple body is needed.  */
  gimple_set_body (decl, body);

  struct function *child_cfun = DECL_STRUCT_FUNCTION (decl);

  /* We need a SESE (single entry - single exit) region to ease the outlining.  */

  basic_block in_bb = create_preheader (loop, CP_FORCE_PREHEADERS);

  edge out = single_exit (loop);

  gcc_assert (out != NULL);

  basic_block out_bb = split_edge (out);

  /* Move the single-entry-single-exit region into its own function.
     We get the new BB in the original function (which returns
     the "factored-out" region.  */
  basic_block new_bb = move_sese_region_to_fn (child_cfun, in_bb,
                                   out_bb, NULL_TREE);

  /* Add call statement into new bb */
  gimple *call_stmt = gimple_build_call_vec (decl, args);
  gimple_stmt_iterator call_iter = gsi_start_bb (new_bb);
  gsi_insert_after (&call_iter, call_stmt, GSI_NEW_STMT);

  /* Add return statement into new exit bb.  */
  gimple *stmt = gimple_build_return (NULL);
  gimple_stmt_iterator gsi = gsi_last_bb(out_bb);
  gsi_insert_after (&gsi, stmt, GSI_NEW_STMT);

  /* Copy function properties */
  child_cfun->curr_properties = cfun->curr_properties;

  /* Create the new function call graph node */
  cgraph_node::get_create (decl);
  /* Make the new function known. */
  cgraph_node::add_new_function (decl, true);

  /* Edge fixup */
  update_max_bb_count ();
  cgraph_edge::rebuild_edges ();

  push_cfun (child_cfun);
  update_max_bb_count ();
  cgraph_edge::rebuild_edges ();

  pop_cfun ();
}

static unsigned int
extract_loop_to_function ()
{
  struct loop *loop;
  expr_vec v = vNULL;

  /* First create the list of loop exits of cfun
     (searches all edges of all bb in cfun).  */
  record_loop_exits ();

  FOR_EACH_LOOP_FN (cfun, loop, LI_ONLY_INNERMOST)
    {
      /* We need single-entry single-exit loops.  */
      if (!single_exit (loop))
	continue;
      if (EDGE_COUNT(loop->header->preds) > 2)
	continue;

      /* Assure the loop is not the entire function.  */
      if ((loop->num_nodes + 5) >= n_basic_blocks_for_fn (cfun))
	continue;

      /* We can't deal with calls inside of loops.  */
      if (contains_call (loop))
	continue;

      /* First we get a unique list of operands in the loop,
         which are defined outside of the loop.  */
      v = collect_def_outside (loop);
      if (v == vNULL)
	continue;

      /* Ignore small loop, which don't offer optimisation potential. */
      if (v.length () < 10)
	continue;

      if (dump_file)
	fprintf (dump_file, "Outlining loop %d\n", loop);

      /* Now we create a new function with the given list of arguments.  */
      outline_loop (loop, v);
    }

  /* Free the list of loop exits of cfun. */
  release_recorded_exits (cfun);

  return 0;
}

namespace {

const pass_data pass_data_noloopalias =
{
  GIMPLE_PASS, /* type */
  "noloopalias", /* name */
  OPTGROUP_NONE, /* optinfo_flags */
  TV_NOLOOPALIAS, /* tv_id */
  PROP_gimple_any, /* properties_required */
  0, /* properties_provided */
  0, /* properties_destroyed */
  0, /* todo_flags_start */
  0, /* todo_flags_finish */
};

class pass_noloopalias: public gimple_opt_pass
{
public:
  pass_noloopalias (gcc::context *ctxt)
    : gimple_opt_pass (pass_data_noloopalias, ctxt)
  {}

  /* opt_pass methods: */
  virtual bool gate (function *fun)
    {
      return flag_noloopalias;
    }

  virtual unsigned int execute (function *)
    {
      return extract_loop_to_function ();
    }
};

} // anon namespace

gimple_opt_pass *
make_pass_noloopalias (gcc::context *ctxt)
{
  return new pass_noloopalias (ctxt);
}
