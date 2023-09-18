/* { dg-options " -O -fstack-protector-strong" } */
/* { dg-final { check-function-bodies "**" "" } } */

void g(void *);

/*
** test1:
**	sub	sp, sp, #304
**	stp	x29, x30, \[sp, #?272\]
**	add	x29, sp, #?272
**	str	(x[0-9]+), \[sp, #?288\]
**	...
**	ldr	(x[0-9]+), \[\1\]
**	str	\2, \[sp, #?264\]
**	mov	\2, *0
**	add	x0, sp, #?8
**	bl	g
**	...
**	ldr	x[0-9]+, \[\1\]
**	...
**	bne	.*
**	...
**	ldr	\1, \[sp, #?288\]
**	ldp	x29, x30, \[sp, #?272\]
**	add	sp, sp, #?304
**	ret
**	bl	__stack_chk_fail
*/
int test1() {
  int y[0x40];
  g(y);
  return 1;
}

/*
** test2:
**	stp	x29, x30, \[sp, #?-32\]!
**	mov	x29, sp
**	str	(x[0-9]+), \[sp, #?16\]
**	sub	sp, sp, #1040
**	...
**	ldr	(x[0-9]+), \[\1\]
**	str	\2, \[sp, #?1032\]
**	mov	\2, *0
**	add	x0, sp, #?8
**	bl	g
**	...
**	ldr	x[0-9]+, \[\1\]
**	...
**	bne	.*
**	...
**	add	sp, sp, #?1040
**	ldr	\1, \[sp, #?16\]
**	ldp	x29, x30, \[sp\], #?32
**	ret
**	bl	__stack_chk_fail
*/
int test2() {
  int y[0x100];
  g(y);
  return 1;
}
