! { dg-do compile }
! { dg-options "-fdec -fno-dec-sequence" }
!
! Test add default SEQUENCE attribute derived types appearing in
! COMMON blocks and EQUIVALENCE statements.
!
! Contributed by Francisco Redondo Marchena <francisco.marchena@codethink.co.uk>
! Modified by Mark Eggleston <mark.eggleston@codethink.com>
!
        MODULE SEQ
          TYPE STRUCT1
            INTEGER*4     ID
            INTEGER*4     TYPE
            INTEGER*8     DEFVAL
            CHARACTER*(4) NAME
            LOGICAL*1     NIL
          END TYPE STRUCT1
        END MODULE

        SUBROUTINE A
          USE SEQ
          TYPE (STRUCT1) S ! { dg-error "Derived type variable" }
          COMMON /BLOCK1/ S
          IF (S%ID.NE.5) STOP 1
          IF (S%TYPE.NE.1000) STOP 2
          IF (S%DEFVAL.NE.-99) STOP 3
          IF (S%NAME.NE."JANE") STOP 4
          IF (S%NIL.NEQV..FALSE.) STOP 5
        END SUBROUTINE

        PROGRAM sequence_att_common
          USE SEQ
          IMPLICIT NONE
          TYPE (STRUCT1) S1
          TYPE (STRUCT1) S2
          TYPE (STRUCT1) S3 ! { dg-error "Derived type variable" }

          EQUIVALENCE (S1,S2) ! { dg-error "Derived type variable" }
          COMMON /BLOCK1/ S3

          S1%ID = 5
          S1%TYPE = 1000
          S1%DEFVAL = -99
          S1%NAME = "JANE"
          S1%NIL = .FALSE.

          IF (S2%ID.NE.5) STOP 1
          IF (S2%TYPE.NE.1000) STOP 2
          IF (S2%DEFVAL.NE.-99) STOP 3
          IF (S2%NAME.NE."JANE") STOP 4
          IF (S2%NIL.NEQV..FALSE.) STOP 5

          S3 = S1

          CALL A
          
        END
