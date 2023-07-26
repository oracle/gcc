! { dg-do compile }
! { dg-options "-fdec -fno-dec-duplicates" }
!
! Test case contributed by Mark Eggleston <mark.eggleston@codethink.com>
!

integer function foo ()
  implicit none
  integer :: x
  integer :: x ! { dg-error "basic type of" }
  x = 42
end function foo
