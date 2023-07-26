! { dg-do compile }
! { dg-options "-std=legacy -flogical-as-integer" }
!
! Test conversion between logical and integer for logical operators
!
        program test
          logical f /.false./
          logical t /.true./
          real x

          x = 7.7
          x = x + t*3.0
          if (abs(x - 10.7).gt.0.00001) stop 1
          x = x + .false.*5.0
          if (abs(x - 10.7).gt.0.00001) stop 2
          x = x - .true.*5.0
          if (abs(x - 5.7).gt.0.00001) stop 3
          x = x + t
          if (abs(x - 6.7).gt.0.00001) stop 4
          x = x + f
          if (abs(x - 6.7).gt.0.00001) stop 5
          x = x - t
          if (abs(x - 5.7).gt.0.00001) stop 6
          x = x - f
          if (abs(x - 5.7).gt.0.00001) stop 7
          x = x**.true.
          if (abs(x - 5.7).gt.0.00001) stop 8
          x = x**.false.
          if (abs(x - 1.0).gt.0.00001) stop 9
          x = x/t
          if (abs(x - 1.0).gt.0.00001) stop 10
          if ((x/.false.).le.huge(x)) stop 11
        end
