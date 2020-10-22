/* { dg-do compile } */
/* { dg-options "-O3 -fipa-cp -fipa-cp-clone -fdump-ipa-cp -findependent-func-asm"  } */

extern int printf (const char *, ...);

static int __attribute__ ((noinline))
foo (int arg)
{
  return 7 * arg;
}

static int __attribute__ ((noinline))
bar (int arg)
{
  return arg * arg;
}

int
baz (int arg)
{
  printf("%d\n", bar (3));
  printf("%d\n", bar (4));
  printf("%d\n", foo (5));
  printf("%d\n", foo (6));
  return foo (7);
}

/* { dg-final { scan-ipa-dump "Function bar.constprop.0" "cp" } } */
/* { dg-final { scan-ipa-dump "Function bar.constprop.1" "cp" } } */
/* { dg-final { scan-ipa-dump "Function foo.constprop.0" "cp" } } */
/* { dg-final { scan-ipa-dump "Function foo.constprop.1" "cp" } } */
/* { dg-final { scan-ipa-dump "Function foo.constprop.2" "cp" } } */
/* { dg-final { scan-ipa-dump-not "Function foo.constprop.3" "cp" } } */
/* { dg-final { scan-ipa-dump-not "Function foo.constprop.4" "cp" } } */
