/* { dg-do compile { target *-*-linux* } } */
/* { dg-options "-O2 -fpic -fno-plt -mindirect-branch=thunk-inline" } */

extern void bar (void);

int
foo (void)
{
  bar ();
  return 0;
}

/* { dg-final { scan-assembler "adrp\[ \t\]*x0, _GLOBAL_OFFSET_TABLE_" } } */
/* { dg-final { scan-assembler "ldr\[ \t\]*x0, \\\[x0, #:gotpage_lo15:bar\\\]" } } */
/* { dg-final { scan-assembler "str\[ \t\]*x30, \\\[sp, #-16\\\]!" } } */
/* { dg-final { scan-assembler "bl\[ \t\]*\.LIND1" } } */
/* { dg-final { scan-assembler "wfe" } } */
/* { dg-final { scan-assembler "b\[ \t\]*\.LIND0" } } */
/* { dg-final { scan-assembler "adr\[ \t\]*x30, .LIND2" } } */
/* { dg-final { scan-assembler "ret" } } */
/* { dg-final { scan-assembler "ldr\[ \t\]*x30, \\\[sp\\\], #16" } } */
/* { dg-final { scan-assembler "blr\[ \t\]*x0" } } */

/* { dg-final { scan-assembler-not "__aarch64_indirect_thunk\n" } } */
