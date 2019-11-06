/* The zero sized bitfield.

   In this testcase, three slices are expected - two of which correspond to
   type int, and one corresponds to unsigned int.  */

/* { dg-do compile )  */
/* { dg-options "-O0 -gt -dA" } */

/* { dg-final { scan-assembler-times "\[\t \]0x2\[\t \]+\[^\n\]*cts_type" 2 } } */
/* { dg-final { scan-assembler-times "\[\t \]0x4\[\t \]+\[^\n\]*cts_type" 1 } } */

/* { dg-final { scan-assembler-times "\[\t \]0x5\[\t \]+\[^\n\]*cts_bits" 1 } } */
/* { dg-final { scan-assembler-times "\[\t \]0xa\[\t \]+\[^\n\]*cts_bits" 1 } } */
/* { dg-final { scan-assembler-times "\[\t \]0\[\t \]+\[^\n\]*cts_bits" 1 } } */

/* { dg-final { scan-assembler-times "ctm_name" 3 } } */
struct foo
{
  int a:5;
  unsigned:0;
  int b:10;
} foome;
