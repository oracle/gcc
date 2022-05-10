/* { dg-do compile } */
/* { dg-options "-O2 -mindirect-branch=thunk-inline -fno-pic" } */

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
/* { dg-final { scan-assembler "str\[ \t\]*x30, \\\[sp, #-16\\\]!" } } */
/* { dg-final { scan-assembler "bl\[ \t\]*\.LIND1" } } */
/* { dg-final { scan-assembler "wfe" } } */
/* { dg-final { scan-assembler "b\[ \t\]*\.LIND0" } } */
/* { dg-final { scan-assembler "adr\[ \t\]*x30, .LIND2" } } */
/* { dg-final { scan-assembler "ret" } } */
/* { dg-final { scan-assembler "ldr\[ \t\]*x30, \\\[sp\\\], #16" } } */
/* { dg-final { scan-assembler "blr\[ \t\]*x1" } } */

/* { dg-final { scan-assembler-not "__aarch64_indirect_thunk" } } */
