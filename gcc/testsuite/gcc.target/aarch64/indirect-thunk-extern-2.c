/* { dg-do compile } */
/* { dg-options "-O2 -mindirect-branch=thunk-extern -fno-pic" } */

typedef void (*dispatch_t)(long offset);

dispatch_t dispatch[256];

void
male_indirect_jump (long offset)
{
  dispatch[offset](offset);
}

/* { dg-final { scan-assembler "adrp\[ \t\]*x1, dispatch" } } */
/* { dg-final { scan-assembler "add\[ \t\]*x1, x1, :lo12:dispatch" } } */
/* { dg-final { scan-assembler "ldr\[ \t\]*x1, \\\[x1, x0, lsl 3\\\]" } } */
/* { dg-final { scan-assembler "b\[ \t\]*__aarch64_indirect_thunk_x1" } } */

/* { dg-final { scan-assembler-not "bl\[ \t\]*\.LIND1" } } */
/* { dg-final { scan-assembler-not "wfe" } } */
/* { dg-final { scan-assembler-not "b\[ \t\]*\.LIND0" } } */
/* { dg-final { scan-assembler-not "adrp\[ \t\]*x30, .LIND2" } } */
/* { dg-final { scan-assembler-not "add\[ \t\]*x30, x30, :lo12:.LIND2" } } */
