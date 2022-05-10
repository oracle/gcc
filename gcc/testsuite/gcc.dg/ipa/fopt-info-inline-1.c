/* { dg-options "-O3 -fopt-info-inline-optimized-missed" } */

static int foo (int a)
{
  return a + 10;
}

static int bar (int b)
{
  return b - 20;
}

static int boo (int a, int b)
{
  return (foo (a)     /* { dg-message "Inlining foo into boo" } */
	  + bar (b)); /* { dg-message "Inlining bar into boo" } */
}

extern int v_a, v_b;
extern int result;

int compute ()
{
  result = boo (v_a, v_b); /* { dg-message "Inlining boo into compute" } */

  return result;
}

extern void not_available(int);

int __attribute__ ((noinline,noclone)) get_input(void)
{
  return 1;
}

int test_1 ()
{
  return get_input (); /* { dg-message "not inlinable: test_1/\[0-9\]+ -> get_input/\[0-9\]+, function not inlinable" } */
}

void test_2 (int v)
{
  not_available (1); /* { dg-message "not inlinable: test_2/\[0-9\]+ -> not_available/\[0-9\]+, function body not available" } */
}
