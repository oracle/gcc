! { dg-do compile }
! { dg-options "-std=legacy -flogical-as-integer -fno-logical-as-integer" }
!
! Based on logical_to_integer_and_vice_versa_1.f but with option disabled
! to test for error messages.
!
! Test case contributed by by Mark Eggleston <mark.eggleston@codethink.com>
!
!
        PROGRAM logical_integer_conversion
          LOGICAL lpos /.true./
          INTEGER ineg/0/
          INTEGER ires
          LOGICAL lres

          ! Test Logicals converted to Integers
          if ((lpos.AND.ineg).EQ.1) STOP 3 ! { dg-error "Operands of logical operator" }
          if ((ineg.AND.lpos).NE.0) STOP 4 ! { dg-error "Operands of logical operator" }
          ires = (.true..AND.0) ! { dg-error "Operands of logical operator" }
          if (ires.NE.0) STOP 5
          ires = (1.AND..false.) ! { dg-error "Operands of logical operator" }
          if (ires.EQ.1) STOP 6

          ! Test Integers converted to Logicals
          if (lpos.EQ.ineg) STOP 7 ! { dg-error "Operands of comparison operator" }
          if (ineg.EQ.lpos) STOP 8 ! { dg-error "Operands of comparison operator" }
          lres = (.true..EQ.0) ! { dg-error "Operands of comparison operator" }
          if (lres) STOP 9
          lres = (1.EQ..false.) ! { dg-error "Operands of comparison operator" }
          if (lres) STOP 10
        END
