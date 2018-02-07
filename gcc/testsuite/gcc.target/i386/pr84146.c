/* PR target/84146 */
/* { dg-do compile } */
/* { dg-options "-O2 -g -mcet -fcf-protection=full" } */

int __setjmp (void **);
void *buf[64];

void
foo (void)
{
  __setjmp (buf);
  for (;;)
    ;
}
