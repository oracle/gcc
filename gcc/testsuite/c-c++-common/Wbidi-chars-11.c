/* PR preprocessor/103026 */
/* { dg-do compile } */
/* { dg-options "-Wbidi-chars=unpaired" } */
/* Test that we warn when mixing UCN and UTF-8.  */

const char *s1 = "LRE_‪_PDF_\u202c";
/* { dg-warning "mismatch" "" { target *-*-* } .-1 } */
const char *s2 = "LRE_\u202a_PDF_‬";
/* { dg-warning "mismatch" "" { target *-*-* } .-1 } */
