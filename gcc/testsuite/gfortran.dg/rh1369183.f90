! { dg-do compile }

module mod1369183
  implicit none
  contains
  subroutine sub(test)
    type test_t
      sequence
      integer(4) type
    end type test_t
    type(test_t),intent(inout) :: test
  end subroutine sub
end module mod1369183
subroutine rh1369183
  use mod1369183
  implicit none 
  type test_t
  sequence
  end type test_t
  type(test_t) :: tst
  call sub(tst)                ! { dg-error "Type mismatch in argument" }
end subroutine rh1369183
