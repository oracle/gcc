{
  for (int c0 = a; c0 <= min(b, a + 9); c0 += 1)
    s0(c0);
  for (int c0 = a + 10; c0 <= min(b, a + 19); c0 += 1) {
    s0(c0);
    s1(c0);
  }
  for (int c0 = max(a + 10, b + 1); c0 <= min(b + 10, a + 19); c0 += 1)
    s1(c0);
  for (int c0 = a + 20; c0 <= b; c0 += 1) {
    s0(c0);
    s1(c0);
    s2(c0);
  }
  for (int c0 = max(a + 20, b + 1); c0 <= b + 10; c0 += 1) {
    s1(c0);
    s2(c0);
  }
  for (int c0 = max(a + 20, b + 11); c0 <= b + 20; c0 += 1)
    s2(c0);
}
