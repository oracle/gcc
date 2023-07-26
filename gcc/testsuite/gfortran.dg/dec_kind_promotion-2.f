!{ dg-do run }
!{ dg-options "-fdec-intrinsic-ints -fdec-promotion" }
!
! integer types of a smaller kind than expected should be
! accepted by type specific intrinsic functions
!
! Contributed by Mark Eggleston <mark.eggleston@codethink.com>
!
      program test_small_type_promtion
        implicit none
        integer(1) :: a = 1
        integer :: i
        if (iiabs(-9_1).ne.9) stop 1
        if (iabs(-9_1).ne.9) stop 2
        if (iabs(-9_2).ne.9) stop 3
        if (jiabs(-9_1).ne.9) stop 4
        if (jiabs(-9_2).ne.9) stop 5
        if (iishft(1_1, 2).ne.4) stop 6
        if (jishft(1_1, 2).ne.4) stop 7
        if (jishft(1_2, 2).ne.4) stop 8
        if (kishft(1_1, 2).ne.4) stop 9
        if (kishft(1_2, 2).ne.4) stop 10
        if (kishft(1_4, 2).ne.4) stop 11
        if (imod(17_1, 3).ne.2) stop 12
        if (jmod(17_1, 3).ne.2) stop 13
        if (jmod(17_2, 3).ne.2) stop 14
        if (kmod(17_1, 3).ne.2) stop 15
        if (kmod(17_2, 3).ne.2) stop 16
        if (kmod(17_4, 3).ne.2) stop 17
        if (inot(5_1).ne.-6) stop 18
        if (jnot(5_1).ne.-6) stop 19
        if (jnot(5_2).ne.-6) stop 20
        if (knot(5_1).ne.-6) stop 21
        if (knot(5_2).ne.-6) stop 22
        if (knot(5_4).ne.-6) stop 23
        if (isign(-77_1, 1).ne.77) stop 24
        if (isign(-77_1, -1).ne.-77) stop 25
        if (isign(-77_2, 1).ne.77) stop 26
        if (isign(-77_2, -1).ne.-77) stop 27
      end program
