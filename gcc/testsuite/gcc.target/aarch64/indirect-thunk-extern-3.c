/* { dg-do compile } */
/* { dg-options "-O2 -mindirect-branch=thunk-extern -fno-pic" } */

typedef void (*dispatch_t)(long offset);

dispatch_t dispatch;

int
male_indirect_jump (long offset)
{
  dispatch(offset);
  return 0;
}

/* { dg-final { scan-assembler "adrp\[ \t\]*x1, dispatch" } } */
/* { dg-final { scan-assembler "ldr\[ \t\]*x1, \\\[x1, #:lo12:dispatch\\\]" } } */
/* { dg-final { scan-assembler "bl\[ \t\]*__aarch64_indirect_thunk_x1" } } */

/* { dg-final { scan-assembler-not "bl\[ \t\]*\.LIND1" } } */
/* { dg-final { scan-assembler-not "wfe" } } */
/* { dg-final { scan-assembler-not "b\[ \t\]*\.LIND0" } } */
/* { dg-final { scan-assembler-not "adrp\[ \t\]*x30, .LIND2" } } */
/* { dg-final { scan-assembler-not "add\[ \t\]*x30, x30, :lo12:.LIND2" } } */
