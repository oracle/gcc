/* CTF does not have representation for some types at this time.  These types
   are skipped in the CTF generation phase in the compiler.
   
   This testcase does NOT cover all skipped types exhaustively.  */

/* { dg-do compile } */
/* { dg-options "-gctf" } */
/* { dg-require-effective-target libc_has_complex_functions } */

#include <complex.h>

typedef float La_x86_64_xmm __attribute__ ((__vector_size__ (16)));
La_x86_64_xmm a1;

/* GCC also supports complex integer data types.  */
complex char a;
complex signed char b;
complex unsigned char c;
complex short int d;
complex short unsigned int e;
complex int f;
complex unsigned int g;
complex long int h;
complex long unsigned int i;
complex long long int j;

enum gomp_schedule_type
{
  GFS_RUNTIME,
  GFS_STATIC,
  GFS_MONOTONIC = 0x80000000U
};

enum gomp_schedule_type gsch_type;
