! { dg-do run }
! { dg-options "-fdec-non-integer-index" }
!
! Test not integer substring indexes
!
! Test case contributed by Mark Eggleston <mark.eggleston@codethink.com>
!
        PROGRAM not_integer_substring_indexes
          CHARACTER*5 st/'Tests'/
          REAL ir/1.0/
          REAL ir2/4.0/

          if (st(ir:4).ne.'Test') stop 1
          if (st(1:ir2).ne.'Test') stop 2
          if (st(1.0:4).ne.'Test') stop 3
          if (st(1:4.0).ne.'Test') stop 4
          if (st(2.5:4).ne.'est') stop 5
        END
