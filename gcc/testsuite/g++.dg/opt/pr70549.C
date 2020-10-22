// PR target/70549
// { dg-do compile }
// { dg-options "-O2" }
// { dg-additional-options "-fPIC" { target fpic } }

struct A { float x; float y; };
A a, b, c;
int d, e;
A bar ();
void foo (A, A);
inline A operator/ (A, A p2) { if (p2.x) return a; }
struct B { A dval; };
int baz (A, B, A, int);

void
test ()
{
  B q;
  A f, g, h, k;
  h.x = 1.0;
  f = h;
  struct A i, j = f;
  do {
    i = bar ();
    g = i / j;
    foo (g, c);
    int l = baz (k, q, b, e);
    if (l)
      goto cleanup;
    j = i;
  } while (d);
cleanup:;
}
