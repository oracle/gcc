! { dg-do compile }
! { dg-options "-fdec -fno-dec-override-kind" }
!
! Test kind specification in variable not in type as the per variable
! kind specification is not enables these should fail
!
! Contributed by Mark Eggleston <mark.eggleston@codethink.com>
!
        program spec_in_var
          integer a
          parameter(a=2)
          integer b*(a) ! { dg-error "Syntax error" }
          real c*(8)    ! { dg-error "Syntax error" }
          logical d*1_1 ! { dg-error "Syntax error" }
        end
