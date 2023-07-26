! { dg-do run }
! { dg-options "-fdec" }
!
! Test kind specification in variable not in type
!
! Contributed by Mark Eggleston <mark.eggleston@codethink.com>
!
        program spec_in_var
          integer a
          parameter(a=2)
          integer b*(a)
          real c*(8)
          logical d*(1_1)
          character e*(a)
          if (kind(b).ne.2) stop 1
          if (kind(c).ne.8) stop 2
          if (kind(d).ne.1) stop 3
          if (len(e).ne.2) stop 4
        end
