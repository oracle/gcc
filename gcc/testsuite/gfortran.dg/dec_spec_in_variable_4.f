! { dg-do compile }
!
! Test kind specification in variable not in type. The per variable
! kind specification is not enabled so these should fail
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
