/* Test compilation of stubs with various qualifiers - const, restrict and
   volatile.

   Testcase includes a std header to allow testing of shared types across
   files.  Only one CTF record for int is expected.
   
   CTF records for CVR qualifiers are no-name records.  In this testcase, there
   are 7 qualifiers across constructs.  2 more no-name CTF records correspond to
   CTF pointer records.

    TYPEID: name string (size) -> ref TYPEID : ref name string (size) -> ...

    b: volatile int (size 4) -> 5: int (size 4)
    c: const volatile int (size 4) -> b: volatile int (size 4) -> 5: int (size 4)
    e: int *restrict (size 8) -> d: int * (size 8) -> 5: int (size 4)
    f: const int (size 4) -> 5: int (size 4)
    10: const int * (size 8) -> f: const int (size 4) -> 5: int (size 4)
    12: int *const (size 8) -> d: int * (size 8) -> 5: int (size 4)
    14: const int *restrict (size 8) -> 10: const int * (size 8) -> f: const int (size 4) -> 5: int (size 4)
    */

/* { dg-do compile )  */
/* { dg-options "-O0 -gt -dA" } */

/* { dg-final { scan-assembler-times "ascii \"int.0\"\[\t \]+\[^\n\]*ctf_string" 1 } } */
/* { dg-final { scan-assembler-times "\[\t \]0\[\t \]+\[^\n\]*ctt_name" 9 } } */
/* { dg-final { scan-assembler-times "\[\t \]0xe000000\[\t \]+\[^\n\]*ctt_info" 2 } } */

#include "stddef.h"

const volatile int a = 5;
int *restrict b;

const int * i;
int const * j;

typedef int * INTP;
const INTP int_p;

void foo (size_t n, int *restrict p, const int *restrict q)
{
  while (n-- > 0)
    *p++ = *q++;
}
