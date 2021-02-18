/* Verify that CTF debug info can co-exist with other debug formats.  */
/* { dg-do compile } */
/* { dg-options "-gctf -dA" } */
/* { dg-final { scan-assembler "0xdff2.*CTF preamble magic number" } } */

void func (void)
{ }
