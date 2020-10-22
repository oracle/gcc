/* { dg-do run } */
/* { dg-options "-O2 -fpic" } */

extern void exit (int);
extern void abort (void);

int b;

struct A
{
  void *pad[147];
  void *ra, *h;
  long o;
};

void
__attribute__((noinline))
foo (struct A *a, void *x)
{
  __builtin_memset (a, 0, sizeof (a));
  if (!b)
    exit (0);
}

void
__attribute__((noinline))
bar (void)
{
  struct A a;

  __builtin_unwind_init ();
  foo (&a, __builtin_return_address (0));
}

int
main (void)
{
  bar ();
  abort ();
  return 0;
}
