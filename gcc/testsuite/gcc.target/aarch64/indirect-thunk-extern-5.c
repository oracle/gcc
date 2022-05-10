/* { dg-do compile { target *-*-linux* } } */
/* { dg-options "-O2 -fpic -fno-plt -mindirect-branch=thunk-extern" } */

extern void bar (void);

void
foo (void)
{
  bar ();
}

/* { dg-final { scan-assembler "adrp\[ \t\]*x0, _GLOBAL_OFFSET_TABLE_" } } */
/* { dg-final { scan-assembler "ldr\[ \t\]*x0, \\\[x0, #:gotpage_lo15:bar\\\]" } } */
/* { dg-final { scan-assembler "b\[ \t\]*__aarch64_indirect_thunk_x0" } } */

/* { dg-final { scan-assembler-not "bl\[ \t\]*\.LIND1" } } */
/* { dg-final { scan-assembler-not "wfe" } } */
/* { dg-final { scan-assembler-not "b\[ \t\]*\.LIND0" } } */
/* { dg-final { scan-assembler-not "adrp\[ \t\]*x30, .LIND2" } } */
/* { dg-final { scan-assembler-not "add\[ \t\]*x30, x30, :lo12:.LIND2" } } */
