! { dg-do compile }
! { dg-options "-fdec" }
!
! Check that invalid kind values are rejected.
!
! Contributed by Mark Eggleston <mark.eggleston@codethink.com>
!
        program spec_in_var
          integer a
          parameter(a=3)
          integer b*(a) ! { dg-error "Kind 3 not supported" }
          real c*(78)   ! { dg-error "Kind 78 not supported" }
          logical d*(*) ! { dg-error "Invalid character" }
        end
