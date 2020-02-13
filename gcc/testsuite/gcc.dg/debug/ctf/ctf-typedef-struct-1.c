/* { dg-do compile )  */
/* { dg-options "-O0 -gt -dA" } */

typedef struct my_int
{
  int upper;
  int lower;
  struct bitmask
    {
      int flags;
    } my_mask;
} my_int_t;

my_int_t mit;
