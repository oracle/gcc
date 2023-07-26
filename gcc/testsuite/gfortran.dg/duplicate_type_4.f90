! { dg-do compile }
! { dg-options "-std=f95" }

! PR fortran/30239
! Check for errors when a symbol gets declared a type twice, even if it
! is the same.

INTEGER FUNCTION foo ()
  IMPLICIT NONE
  INTEGER :: x
  INTEGER :: x ! { dg-error "basic type of" }
  x = 42
END FUNCTION foo
