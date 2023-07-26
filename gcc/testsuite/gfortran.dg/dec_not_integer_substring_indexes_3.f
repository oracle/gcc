! { dg-do compile }
! { dg-options "-fdec -fno-dec-non-integer-index" }
!
! Test not integer substring indexes
!
! Test case contributed by Mark Eggleston <mark.eggleston@codethink.com>
!
        PROGRAM not_integer_substring_indexes
          CHARACTER*5 st/'Tests'/
          REAL ir/1.0/
          REAL ir2/4.0/

          if (st(ir:4).ne.'Test') stop 1 ! { dg-error "Substring start index" }
          if (st(1:ir2).ne.'Test') stop 2 ! { dg-error "Substring end index" }
          if (st(1.0:4).ne.'Test') stop 3 ! { dg-error "Substring start index" }
          if (st(1:4.0).ne.'Test') stop 4 ! { dg-error "Substring end index" }
          if (st(2.5:4).ne.'est') stop 5 ! { dg-error "Substring start index" }
        END
