! { dg-do run }
! { dg-options "-fdec-non-logical-if -Wconversion-extra" }
!
! Allow logical expressions in if statements and blocks
!
! Contributed by Francisco Redondo Marchena <francisco.marchema@codethink.co.uk>
!             and Jeff Law <law@redhat.com>
! Modified by Mark Eggleston <mark.eggleston@codethink.com>
!
        PROGRAM logical_exp_if_st_bl
          INTEGER ipos/1/
          INTEGER ineg/0/

          ! Test non logical variables
          if (ineg) STOP 1 ! { dg-warning "if it evaluates to nonzero" }
          if (0) STOP 2 ! { dg-warning "if it evaluates to nonzero" }

          ! Test non logical expressions in if statements
          if (MOD(ipos, 1)) STOP 3 ! { dg-warning "if it evaluates to nonzero" }

          ! Test non logical expressions in if blocks
          if (MOD(2 * ipos, 2)) then ! { dg-warning "if it evaluates to nonzero" }
            STOP 4
          endif
        END
