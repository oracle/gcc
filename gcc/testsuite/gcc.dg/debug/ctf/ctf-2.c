/* A LEVEL of 0 with -gt turns off CTF debug info generation.  */

/* { dg-do compile } */
/* { dg-options "-gt0 -dA" } */

/* { dg-final { scan-assembler-times "0xdff2.*CTF preamble magic number" 0} } */
/* { dg-final { scan-assembler-times "0x4.*CTF preamble version" 0 } } */
/* { dg-final { scan-assembler-times "0.*CTF preamble flags" 0 } } */

const char * _CONTEXT_NAME = "foobar";
