! { dg-do run }
! { dg-options "-std=legacy -flogical-as-integer" }
!
! Test conversion between logical and integer for logical operators
!
! Test case contributed by Jim MacArthur <jim.macarthur@codethink.co.uk>
! Modified for -flogical-as-integer by Mark Eggleston
! <mark.eggleston@codethink.com>
!
        PROGRAM logical_integer_conversion
          LOGICAL lpos /.true./
          INTEGER ineg/0/
          INTEGER ires
          LOGICAL lres

          ! Test Logicals converted to Integers
          if ((lpos.AND.ineg).EQ.1) STOP 3
          if ((ineg.AND.lpos).NE.0) STOP 4
          ires = (.true..AND.0)
          if (ires.NE.0) STOP 5
          ires = (1.AND..false.)
          if (ires.EQ.1) STOP 6

          ! Test Integers converted to Logicals
          if (lpos.EQ.ineg) STOP 7
          if (ineg.EQ.lpos) STOP 8
          lres = (.true..EQ.0)
          if (lres) STOP 9
          lres = (1.EQ..false.)
          if (lres) STOP 10
        END
