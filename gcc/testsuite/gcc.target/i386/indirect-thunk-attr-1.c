/* { dg-do compile } */
/* { dg-options "-O2 -fno-pic" } */

typedef void (*dispatch_t)(long offset);

dispatch_t dispatch;

extern void male_indirect_jump (long)
  __attribute__ ((indirect_branch("thunk")));

void
male_indirect_jump (long offset)
{
  dispatch(offset);
}

/* Our gcc-4.8 based compiler is not as aggressive at sibcalls
   where the target is in a MEM.  Thus we have to scan for different
   patterns here than in newer compilers.  */
/* { dg-final { scan-assembler "mov(?:l|q)\[ \t\]*_?dispatch" } } */
/* { dg-final { scan-assembler "jmp\[ \t\]*__x86_indirect_thunk_(r|e)ax" } } */
/* { dg-final { scan-assembler "jmp\[ \t\]*\.LIND" } } */
/* { dg-final { scan-assembler "call\[ \t\]*\.LIND" } } */
/* { dg-final { scan-assembler {\tpause} } } */
/* { dg-final { scan-assembler {\tlfence} } } */
