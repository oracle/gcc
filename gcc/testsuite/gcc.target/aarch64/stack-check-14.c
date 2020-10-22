/* { dg-do compile } */
/* { dg-options "-O2 -fstack-clash-protection --param stack-clash-protection-guard-size=12" } */
/* { dg-require-effective-target supports_stack_clash_protection } */

int t1(int);

int t2(int x)
{
  char *p = __builtin_alloca (4050);
  x = t1 (x);
  return p[x];
}


/* This test has a constant sized alloca that is smaller than the
   probe interval.  But it actually requires two probes instead
   of one because of the optimistic assumptions we made in the
   aarch64 prologue code WRT probing state. 

   The form can change quite a bit so we just check for two
   probes without looking at the actual address.  */
/* { dg-final { scan-assembler-times "str\\txzr," 2 } } */



