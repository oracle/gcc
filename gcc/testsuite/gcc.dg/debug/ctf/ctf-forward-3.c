/* A compilation unit may have a forward declation of a type and the definition
   of the type at a later point.

   CTF forward type is generated for forward declarations of types in C.

   Once the type is defined, the CTF generated for the forward declaration is
   updated to non-root type.
   
   In this testcase, one CTF_K_FORWARD type and two CTF_K_STRUCT types are
   expected.  */

/* { dg-do compile )  */
/* { dg-options "-O0 -gt -dA" } */

/* { dg-final { scan-assembler-times "\[\t \]0x24000000\[\t \]+\[^\n\]*ctt_info" 1 } } */
/* { dg-final { scan-assembler-times "\[\t \]0x6\[\t \]+\[^\n\]*ctt_size or ctt_type" 1 } } */
/* { dg-final { scan-assembler-times "\[\t \]0x8\[\t \]+\[^\n\]*ctt_size or ctt_type" 2 } } */
/* { dg-final { scan-assembler-times "ascii \"afoo.0\"\[\t \]+\[^\n\]*ctf_string" 1 } } */
/* { dg-final { scan-assembler-times "ascii \"bfoo.0\"\[\t \]+\[^\n\]*ctf_string" 1 } } */
struct bar { struct foo * a; } barbar;
struct foo { int afoo; int bfoo; } foofoo;
