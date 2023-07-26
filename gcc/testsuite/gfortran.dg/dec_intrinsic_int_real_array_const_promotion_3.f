! { dg-do compile }
! { dg-options "-fdec -fno-dec-promotion" }
!
! Test promotion between integers and reals for mod and modulo where
! A is a constant array and P is zero.
!
! Compilation errors are expected
!
! Contributed by Francisco Redondo Marchena <francisco.marchena@codethink.co.uk>
!             and Jeff Law <law@redhat.com>
! Modified by Mark Eggleston <mark.eggleston@codethink.com>
!
      program promotion_int_real_array_const
          real a(2) = mod([12, 34], 0.0)*4    ! { dg-error "'a' and 'p' arguments of 'mod'" }
          a = mod([12.0, 34.0], 0)*4          ! { dg-error "'a' and 'p' arguments of 'mod'" }
          real b(2) = modulo([12, 34], 0.0)*4 ! { dg-error "'a' and 'p' arguments of 'modulo'" }
          b = modulo([12.0, 34.0], 0)*4       ! { dg-error "'a' and 'p' arguments of 'modulo'" }
      end program
