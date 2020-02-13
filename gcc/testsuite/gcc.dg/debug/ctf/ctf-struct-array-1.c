/* Test Compilation of mixed constructs containing structs and arrays.

   Further, the compiler is expected to generate a single CTF struct type for
   struct cmodel (due to Type de-duplication at CTF generation).
   
   const qualifiers in fields of structs should be processed.  It appears as a
   no-name CTF record with appropriate ctt_info.  In this testcase, 2 const
   qualifier records are expected - const char and const struct cmodel.  */

/* { dg-do compile )  */
/* { dg-options "-O0 -gt -dA" } */
/* { dg-final { scan-assembler-times "ascii \"cmodel.0\"\[\t \]+\[^\n\]*ctf_string" 1 } } */
/* { dg-final { scan-assembler-times "ascii \"cname.0\"\[\t \]+\[^\n\]*ctf_string" 1 } } */
/* { dg-final { scan-assembler-times "ascii \"cpointer.0\"\[\t \]+\[^\n\]*ctf_string" 1 } } */
/* { dg-final { scan-assembler-times "ascii \"cmodel_t.0\"\[\t \]+\[^\n\]*ctf_string" 1 } } */

/* { dg-final { scan-assembler-times "\[\t \]0x32000000\[\t \]+\[^\n\]*ctt_info" 2 } } */

struct a
{
  int a1[2];
  struct { int b[3]; } a2;
};

struct a my_a;

typedef struct cmodel
{
  const char *cname;
  int ccode;
  int cpointer;
} cmodel_t;

static const cmodel_t _models[] = {
  {"ILP32", 0, 4},
  {"LP64", 0, 8},
  {"", 0, 0}
};
