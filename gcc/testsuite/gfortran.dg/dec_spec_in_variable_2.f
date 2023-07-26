! { dg-do run }
! { dg-options "-fdec-override-kind" }
!
! Test kind specification in variable not in type
!
! Contributed by Mark Eggleston <mark.eggleston@codethink.com>
!
        program spec_in_var
          integer*8  ai*1, bi*4, ci
          real*4 ar*4, br*8, cr

          ai = 1
          ar = 1.0
          bi = 2
          br = 2.0
          ci = 3
          cr = 3.0

          if (ai .ne. 1) stop 1
          if (abs(ar - 1.0) > 1.0D-6) stop 2
          if (bi .ne. 2) stop 3
          if (abs(br - 2.0) > 1.0D-6) stop 4
          if (ci .ne. 3) stop 5
          if (abs(cr - 3.0) > 1.0D-6) stop 6
          if (kind(ai) .ne. 1) stop 7
          if (kind(ar) .ne. 4) stop 8
          if (kind(bi) .ne. 4) stop 9
          if (kind(br) .ne. 8) stop 10
          if (kind(ci) .ne. 8) stop 11
          if (kind(cr) .ne. 4) stop 12
        end
