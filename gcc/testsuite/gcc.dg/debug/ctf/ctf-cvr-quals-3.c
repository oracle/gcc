/* Test compilation of stubs with various expressions involving const and
   volatile qualifiers.
   
   In this testcase, it is expected to have const and volatile CTF records,
   some of which are shared or referenced (shown by '->' below) :

   Types:
    1: unsigned char (size 1)
    2: volatile unsigned char (size 1) -> 1: unsigned char (size 1)
    3: const volatile unsigned char (size 1) -> 2: volatile unsigned char (size 1) -> 1: unsigned char (size 1)
    4: const volatile unsigned char * (size 8) -> 3: const volatile unsigned char (size 1) -> 2: volatile unsigned char (size 1) -> 1: unsigned char (size 1) [POINTER_TYPE]
    5: int (size 4)
    6: float (size 4)
    7: volatile float (size 4) -> 6: float (size 4)
    8: volatile float [2] (size 8) [ARRAY_TYPE]
    9: char (size 1)
    a: volatile char (size 1) -> 9: char (size 1)
    b: const volatile char (size 1) -> a: volatile char (size 1) -> 9: char (size 1)
    c: const volatile char [2] (size 2) [ARRAY_TYPE]

    Types 2, 3, 7, a, and b are CTF types of kind CTF_K_CONST or CTF_K_VOLATILE.  */

/* { dg-do compile )  */
/* { dg-options "-O0 -gt -dA" } */

/* { dg-final { scan-assembler-times "ascii \"int.0\"\[\t \]+\[^\n\]*ctf_string" 1 } } */
/* { dg-final { scan-assembler-times "\[\t \]0x2e000000\[\t \]+\[^\n\]*ctt_info" 3 } } */
/* { dg-final { scan-assembler-times "\[\t \]0x32000000\[\t \]+\[^\n\]*ctt_info" 2 } } */
/* Two arrays.  */
/* { dg-final { scan-assembler-times "\[\t \]0x12000000\[\t \]+\[^\n\]*ctt_info" 2 } } */


const volatile unsigned char  vicar = 11;

const volatile unsigned char * vicarage = &vicar;

volatile float vilify[2];

const volatile char victor = 'Y';

const volatile char vindictive[2];
