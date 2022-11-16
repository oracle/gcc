#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "backend.h"
#include "hash-set.h"
#include "vec.h"
#include "input.h"
#include "double-int.h"
#include "alias.h"
#include "symtab.h"
#include "wide-int.h"
#include "inchash.h"
#include "tm.h"
#include "hard-reg-set.h"
#include "predict.h"
#include "function.h"
#include "dominance.h"
#include "predict.h"
#include "tree.h"
#include "rtl.h"
#include "df.h"
#include "memmodel.h"
#include "emit-rtl.h"
#include "tree-pass.h"
#include "insn-config.h"
#include "recog.h"
#include "cfgrtl.h"
#include "rtl-iter.h"

static int num_changes;
static int num_mul_changes;

static bool optimize_this_for_speed_p;

static void
move_dead_notes (rtx_insn *to_insn, rtx_insn *from_insn, rtx expr)
{
  rtx note;
  rtx next_note;
  rtx prev_note = NULL;
  rtx datum;

  for (note = REG_NOTES (from_insn); note; note = next_note)
    {
      next_note = XEXP (note, 1);
      datum = XEXP (note, 0);

      if (REG_NOTE_KIND (note) == REG_DEAD && rtx_referenced_p (datum, expr))
        {
          XEXP (note, 1) = REG_NOTES (to_insn);
          REG_NOTES (to_insn) = note;
          if (prev_note)
            XEXP (prev_note, 1) = next_note;
          else
            REG_NOTES (from_insn) = next_note;
        }
      else prev_note = note;
    }
}

/* checks if USE was modified between DEF_INSN and TARGET_INSN
   (both exclusive). This is trivial to determine if both insns are
   in the same basic block. If not they need to be in the same extended
   basic block. Other cases are regarded as if USE was modified.  */
static bool
use_killed_between(rtx use, rtx_insn *def_insn, rtx_insn *target_insn)
{
  basic_block def_bb = BLOCK_FOR_INSN (def_insn);
  basic_block target_bb = BLOCK_FOR_INSN (target_insn);

  if (def_bb == target_bb)
    {
      /* There are cases where the use is *before* the definition.
         E.g. uninitialized variables.  */
      if (DF_INSN_LUID (def_insn) >= DF_INSN_LUID (target_insn))
        return true;

      else
        return modified_between_p (use, def_insn, target_insn);
    }

  else {
    basic_block loop_bb = target_bb;

    if (modified_in_p (use, BB_HEAD (target_bb))
        || modified_between_p (use, BB_HEAD (target_bb), target_insn))
      return true;

    while (single_pred_p (loop_bb)) {
      loop_bb = single_pred (loop_bb);

      if (loop_bb == def_bb)
        return modified_between_p (use, def_insn, BB_END (def_bb))
               || modified_in_p (use, BB_END (def_bb));
      else if (modified_in_p (use, BB_HEAD (loop_bb))
               || modified_between_p (use, BB_HEAD (loop_bb), BB_END (loop_bb))
               || modified_in_p (use, BB_END (loop_bb)))
        return true;
    }

    return true;
  }
}

/* determine costs for INSN. Invokes `set_src_cost` for both SRC
   and DEST of the SET insn and adds both costs. This is necessary
   since `insn_rtx_costs` only returns the costs of the SRC but does
   not factor in the costs for the DEST.  */
static int
insn_cost (rtx_insn *insn)
{
  int cost = set_rtx_cost (PATTERN (insn), optimize_this_for_speed_p);

  if (dump_file && cost > 20)
    {
      fprintf (dump_file, "LARGE COST FOR INSN: %d\n", cost);
      print_rtl_single (dump_file, insn);
    }

  return cost;
}

/* returns TRUE if DEST_REG in USE_INSN could be replaced with its
   initialization value.  */
static bool
try_prop_def_to_use (rtx_insn *def_insn, rtx dest_reg, df_ref use, rtx *loc)
{
  rtx src = SET_SRC (PATTERN (def_insn));
  rtx_insn *use_insn = DF_REF_INSN (use);

  if (DF_REF_FLAGS (use) & DF_REF_READ_WRITE)
    return false;

  if (DF_REF_IS_ARTIFICIAL (use))
    return false;

  rtx use_set = single_set (use_insn);

  /* insn needs to be either set or debug_insn.  */
  if (!use_set && !DEBUG_INSN_P (use_insn))
    return false;

  rtx reg = DF_REF_REG (use);

  /* only move if usage if reg is register with same mode.  */
  if (!REG_P (reg) || GET_MODE (reg) != GET_MODE (dest_reg))
    return false;

  /* do not forward propagate into loop.  */
  if (BLOCK_FOR_INSN (def_insn)->loop_father != BLOCK_FOR_INSN (use_insn)->loop_father)
    return false;

  df_ref use_iterator;

  /* cancel for multiple definitions.  */
  FOR_EACH_INSN_INFO_USE (use_iterator, DF_INSN_INFO_GET (use_insn))
    {
      if (DF_REF_REGNO (use_iterator) == REGNO (dest_reg))
        {
          df_link *chain = DF_REF_CHAIN (use_iterator);

          if (chain->next)
            return false;
        }
    }

  /* register needs to occur in use_set.  */
  if (use_set && !reg_mentioned_p (DF_REF_REG (use), SET_SRC (use_set)))
    return false;

  /* check if src was modified between def_insn and use_insn.  */
  if (!CONSTANT_P (src) && use_killed_between (src, def_insn, use_insn))
    return false;


  int old_costs = insn_cost (use_insn);

  validate_unshare_change (use_insn, loc, src, true);

  if (!DEBUG_INSN_P (use_insn))
    {
      int code = recog_memoized (use_insn);

      /* check if instruction wasn't recognized */
      if (code == -1)
        {
          if (dump_file && (dump_flags & TDF_DETAILS))
            {
              fprintf (dump_file, "insn not recognized:\n");
              print_rtl_single (dump_file, use_insn);
            }

          return false;
        }

      int new_costs = insn_cost (use_insn);

      /* increase current costs */
      return new_costs <= old_costs;
    }

  return true;
}


static struct df_link *
get_uses (rtx_insn *insn, rtx reg)
{
  df_ref def;

  FOR_EACH_INSN_DEF (def, insn)
    {
      if (REGNO (DF_REF_REG (def)) == REGNO (reg))
        break;
    }

  gcc_assert (def != NULL);

  return DF_REF_CHAIN (def);
}

static bool
insn_later(rtx_insn *cur, rtx_insn *last) {
  basic_block cur_bb = BLOCK_FOR_INSN (cur);
  basic_block last_bb = BLOCK_FOR_INSN (last);

  if (cur_bb == last_bb)
      return DF_INSN_LUID (cur) > DF_INSN_LUID (last);

  /* if insn's are in two different basic blocks, we can assume that one
     has the other as single predecessor. Return true if cur insn is after
     last insn, that means if cur_bb has last_bb as its single predecessor.  */
  else
    {
      basic_block loop_bb = cur_bb;

      while (single_pred_p (loop_bb))
        {
          loop_bb = single_pred (loop_bb);

          if (loop_bb == last_bb)
            return true;
        }

      return false;
    }
}

static void
dump_uncse (rtx_insn *def_insn)
{
  fprintf (dump_file, "uncse def\n");
  print_rtl_single (dump_file, def_insn);

  struct df_link *ref_chain, *ref_link;

  fprintf (dump_file, "into\n");
  bool first = true;

  rtx dest = SET_DEST (PATTERN (def_insn));

  ref_chain = get_uses (def_insn, dest);
  for (ref_link = ref_chain; ref_link; ref_link = ref_link->next)
    if (DF_REF_INSN_INFO (ref_link->ref))
      {
        rtx_insn *target_insn = DF_REF_INSN (ref_link->ref);

        if (!first)
          fprintf (dump_file, "and\n");

        print_rtl_single (dump_file, target_insn);
        first = false;
      }
}

/* returns TRUE if DEF_INSN could be forwarded into all
   its uses and hence was deleted.  */
static bool
try_uncse_def (rtx_insn *def_insn)
{
  /* only handle SET */
  if (!INSN_P(def_insn)
      || GET_CODE (PATTERN (def_insn)) != SET)
    return false;

  rtx dest = SET_DEST (PATTERN (def_insn));

  /* for now: only handle assignments to register */
 if (!REG_P (dest) || GET_MODE (dest) == VOIDmode)
    return false;

  rtx src = SET_SRC (PATTERN (def_insn));

  /* do not care about register to register assignments */
  if (REG_P (src))
    return false;

  /* do not propagate definition with side effects */
  if (side_effects_p (src))
    return false;

  /* ignore definition if source contains memory access */
  subrtx_iterator::array_type array;
  FOR_EACH_SUBRTX (iter, array, src, ALL)
    if (MEM_P (*iter) && !MEM_READONLY_P (*iter))
      return false;

  struct df_link *ref_chain, *ref_link;
  rtx_insn *last_insn = NULL;
  bool replaced_all = true;
  int number_replaced = 0;

  ref_chain = get_uses (def_insn, dest);
  for (ref_link = ref_chain; ref_link; ref_link = ref_link->next)
    {
      if (DF_REF_INSN_INFO (ref_link->ref))
        {
          df_ref use = ref_link->ref;
          rtx_insn *target_insn = DF_REF_INSN (use);

          if (!target_insn->deleted()
              && try_prop_def_to_use (def_insn, dest, use, DF_REF_LOC (use)))
            {
              number_replaced++;

              /* remember last use instruction for REG_DEAD-movement.  */
              if (!last_insn || insn_later(target_insn, last_insn))
                last_insn = target_insn;

              continue;
            }
        }

      replaced_all = false;
      break;
    }

  if (replaced_all && apply_change_group ())
    {
      if (last_insn)
        move_dead_notes (last_insn, def_insn, src);

      num_changes++;

      if (number_replaced > 1)
        num_mul_changes++;

      if (dump_file)
        dump_uncse (def_insn);

      delete_insn_and_edges (def_insn);

      return true;
    }

  cancel_changes (0);
  return false;
}

/* move through all insns and try to forward a definition
   into all its uses. Start of the uncse-pass.  */
static void
find_uncse_poss()
{
  num_changes = 0;
  num_mul_changes = 0;

  df_chain_add_problem (DF_UD_CHAIN + DF_DU_CHAIN);
  df_analyze ();
  df_set_flags (DF_DEFER_INSN_RESCAN);

  basic_block bb;
  rtx_insn *insn;

  int cleanup = 0;
  const char *fname = current_function_name();

  FOR_EACH_BB_FN (bb, cfun)
    {
      optimize_this_for_speed_p = optimize_bb_for_speed_p (bb);

      FOR_BB_INSNS (bb, insn)
        cleanup |= try_uncse_def (insn);
    }

  if (dump_file)
    fprintf (dump_file,
	     "\nNumber changes in %s: %d %d (with multiple uses)\n\n",
	     fname, num_changes, num_mul_changes);
}

static unsigned int
rest_of_handle_uncse ()
{
  find_uncse_poss();
  return 0;
}

namespace {

const pass_data pass_data_uncse =
{
  RTL_PASS, /* type */
  "uncse", /* name */
  OPTGROUP_NONE, /* optinfo_flags */
  TV_UNCSE, /* tv_id */
  PROP_cfglayout, /* properties_required */
  0, /* properties_provided */
  0, /* properties_destroyed */
  0, /* todo_flags_start */
  TODO_df_finish, /* todo_flags_finish */
};

class pass_uncse : public rtl_opt_pass
{
public:
  pass_uncse (gcc::context *ctxt)
    : rtl_opt_pass (pass_data_uncse, ctxt)
  {}

  /* opt_pass methods: */
  virtual bool gate (function *) { return optimize > 0 && flag_uncse; }

  virtual unsigned int execute (function *)
    {
      return rest_of_handle_uncse ();
    }

}; // class pass_uncse

} // anon namespace

rtl_opt_pass *
make_pass_uncse (gcc::context *ctxt)
{
  return new pass_uncse (ctxt);
}
