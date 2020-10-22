/* { dg-do compile } */
/* { dg-options "-O2 -mno-indirect-branch-register -mfunction-return=keep -mindirect-branch=thunk-inline -fno-pic" } */

typedef void (*dispatch_t)(long offset);

dispatch_t dispatch[256];

void
male_indirect_jump (long offset)
{
  dispatch[offset](offset);
}

/* Our gcc-4.8 based compiler is not as aggressive at sibcalls
   where the target is in a MEM.  Thus we have to scan for different
   patterns here than in newer compilers.  */
/* { dg-final { scan-assembler "mov(?:l|q)\[ \t\]*_?dispatch" } } */
/* { dg-final { scan-assembler "jmp\[ \t\]*\.LIND" } } */
/* { dg-final { scan-assembler "call\[ \t\]*\.LIND" } } */
/* { dg-final { scan-assembler {\tpause} } } */
/* { dg-final { scan-assembler {\tlfence} } } */
/* { dg-final { scan-assembler-not "__x86_indirect_thunk" } } */
/* { dg-final { scan-assembler-not "pushq\[ \t\]%rax" { target x32 } } } */
