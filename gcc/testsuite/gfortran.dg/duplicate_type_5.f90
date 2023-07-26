! { dg-do run }
! { dg-options "-fdec" }
!
! Test case contributed by Mark Eggleston <mark.eggleston@codethink.com>
!

program test
  implicit none
  integer :: x
  integer :: x
  x = 42
  if (x /= 42) stop 1
end program test
