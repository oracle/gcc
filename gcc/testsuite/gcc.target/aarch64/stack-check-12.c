/* { dg-do compile } */
/* { dg-options "-O2 -fstack-clash-protection --param stack-clash-protection-guard-size=12" } */
/* { dg-require-effective-target supports_stack_clash_protection } */

extern void arf (unsigned long int *, unsigned long int *);
void
frob ()
{
  unsigned long int num[1000];
  unsigned long int den[1000];
  arf (den, num);
}

/* This verifies that the scheduler did not break the dependencies
   by adjusting the offsets within the probe and that the scheduler
   did not reorder around the stack probes.  */
/* { dg-final { scan-assembler-times "sub\\tsp, sp, #4096\\n\\tstr\\txzr, .sp, 4088." 3 } } */



