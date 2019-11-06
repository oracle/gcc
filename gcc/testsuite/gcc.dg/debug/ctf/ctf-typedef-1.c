/* CTF_K_TYPEDEF record generation.

   In this testcase, 7 typedef records are expected.  */

/* { dg-do compile )  */
/* { dg-options "-O0 -gt -dA" } */

/* { dg-final { scan-assembler-times "0x2a000000\[\t \]+\[^\n\]*ctt_info" 7 } } */

typedef int my_int;
typedef int bar_int;
typedef int foo_int;

typedef struct { int a[2]; } my_array;

typedef const int CINT;
typedef const int * CINTP;

typedef struct my_node
{
  int flags;
  char value;
} my_node_t;
