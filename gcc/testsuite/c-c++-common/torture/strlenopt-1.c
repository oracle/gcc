/* { dg-do run } */
/* { dg-options "-fno-early-inlining" } */

#define FORTIFY_SOURCE 2

struct S {
 char skip;
 char name[0];
};

static char static_buf[4];

static void
print_name_len(void *p)
{
  struct S *q = (struct S *) p;
  if (__builtin_strlen(q->name) != 2)
    __builtin_abort ();
}

int
main(void)
{
  // treat static storage as struct
  struct S *c = (struct S *)static_buf;
  __builtin_strcpy(c->name, "aa");

  // copy static storage to stack storage
  char stack_buf[4] = { 0 };
  __builtin_memcpy(stack_buf, static_buf, 4);

  // static and stack both now contain ( 0, 'a', 'a', 0 }

  // indirectly pass the stack storage to the length function
  char *s = (char *)stack_buf;
  print_name_len(s);
  return 0;
}
