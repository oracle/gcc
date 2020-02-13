/* Test compilation of stubs with various qualifiers - const, restrict and
   volatile.

   Testcase includes a std header to allow testing of shared types across
   files.  Only one CTF record for int is expected.
   
   CTF records for CVR qualifiers are no-name records.  In this testcase, there
   are 6 qualifiers across constructs.  2 more no-name CTF records correspond to
   CTF pointer records.

    TYPEID: name string (size) -> ref TYPEID : ref name string (size) -> ...

   Types:
      1: int (size 0x4)
      2: volatile int (size 0x4) -> 1: int (size 0x4)
      3: const volatile int (size 0x4) -> 2: volatile int (size 0x4) -> 1: int (size 0x4)
      4: int * (size 0x8) -> 1: int (size 0x4)
      5: int *restrict (size 0x8) -> 4: int * (size 0x8) -> 1: int (size 0x4)
      6: const int (size 0x4) -> 1: int (size 0x4)
      7: const int * (size 0x8) -> 6: const int (size 0x4) -> 1: int (size 0x4)
      8: int *const (size 0x8) -> 4: int * (size 0x8) -> 1: int (size 0x4)
      9: INTP (size 0x8) -> 8: int *const (size 0x8) -> 4: int * (size 0x8) -> 1: int (size 0x4)
      a: void (size 0x0)
      b: long unsigned int (size 0x8)
      c: const int *restrict (size 0x8) -> 7: const int * (size 0x8) -> 6: const int (size 0x4) -> 1: int (size 0x4)
      d: void () (size 0x0)
    */

/* { dg-do compile )  */
/* { dg-options "-O0 -gt -dA" } */

/* { dg-final { scan-assembler-times "ascii \"int.0\"\[\t \]+\[^\n\]*ctf_string" 1 } } */
/* { dg-final { scan-assembler-times "\[\t \]0\[\t \]+\[^\n\]*ctt_name" 8 } } */

/* { dg-final { scan-assembler-times "\[\t \]0xe000000\[\t \]+\[^\n\]*ctt_info" 2 } } */
/* { dg-final { scan-assembler-times "\[\t \]0x32000000\[\t \]+\[^\n\]*ctt_info" 3 } } */

/* type id 4, 7 have POINTER type.  */
/* { dg-final { scan-assembler-times "\[\t \]0xe000000\[\t \]+\[^\n\]*ctt_info" 2 } } */

/* type id 3, 6, 8 have CONST qualifier.  */
/* { dg-final { scan-assembler-times "\[\t \]0x32000000\[\t \]+\[^\n\]*ctt_info" 3 } } */

/* type id 5,c have RESTRICT qualifier.  */
/* { dg-final { scan-assembler-times "\[\t \]0x36000000\[\t \]+\[^\n\]*ctt_info" 2 } } */

/* type id 2 has VOLATILE qualifier.  */
/* { dg-final { scan-assembler-times "\[\t \]0x2e000000\[\t \]+\[^\n\]*ctt_info" 1 } } */

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
