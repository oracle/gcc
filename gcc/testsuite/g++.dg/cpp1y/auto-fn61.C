// PR c++/88003
// { dg-do compile { target c++14 } }
// { dg-additional-options "-fchecking -fno-report-bug" }
// { dg-ice "poplevel_class" }

auto test() {
  struct O {
    struct N;
  };
  return O();
}

typedef decltype(test()) TN;
struct TN::N {};
