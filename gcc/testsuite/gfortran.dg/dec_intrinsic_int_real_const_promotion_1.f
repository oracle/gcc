! { dg-do run }
! { dg-options "-fdec -finit-real=snan" }
!
! Test promotion between integers and reals in intrinsic operations.
! These operations are: mod, modulo, dim, sign, min, max, minloc and
! maxloc.
!
! Contributed by Francisco Redondo Marchena <francisco.marchena@codethink.co.uk>
!             and Jeff Law <law@redhat.com>
! Modified by Mark Eggleston <mark.eggleston@codethink.com>
!
      PROGRAM promotion_int_real_const
        ! array_nan 4th position value is NAN
        REAL array_nan(4)
        DATA array_nan(1)/-4.0/
        DATA array_nan(2)/3.0/
        DATA array_nan(3)/-2/

        INTEGER m_i/0/
        REAL m_r/0.0/

        INTEGER md_i/0/
        REAL md_r/0.0/

        INTEGER d_i/0/
        REAL d_r/0.0/

        INTEGER s_i/0/
        REAL s_r/0.0/

        INTEGER mn_i/0/
        REAL mn_r/0.0/

        INTEGER mx_i/0/
        REAL mx_r/0.0/

        m_i = MOD(4, 3)
        if (m_i .ne. 1) STOP 1
        m_r = MOD(4.0, 3.0)
        if (abs(m_r - 1.0) > 1.0D-6) STOP 2
        m_r = MOD(4, 3.0)
        if (abs(m_r - 1.0) > 1.0D-6) STOP 3
        m_r = MOD(4.0, 3)
        if (abs(m_r - 1.0) > 1.0D-6) STOP 4

        md_i = MODULO(4, 3)
        if (md_i .ne. 1) STOP 5
        md_r = MODULO(4.0, 3.0)
        if (abs(md_r - 1.0) > 1.0D-6) STOP 6
        md_r = MODULO(4, 3.0)
        if (abs(md_r - 1.0) > 1.0D-6) STOP 7
        md_r = MODULO(4.0, 3)
        if (abs(md_r - 1.0) > 1.0D-6) STOP 8

        d_i = DIM(4, 3)
        if (d_i .ne. 1) STOP 9
        d_r = DIM(4.0, 3.0)
        if (abs(d_r - 1.0) > 1.0D-6) STOP 10
        d_r = DIM(4.0, 3)
        if (abs(d_r - 1.0) > 1.0D-6) STOP 11
        d_r = DIM(3, 4.0)
        if (abs(d_r) > 1.0D-6) STOP 12

        s_i = SIGN(-4, 3)
        if (s_i .ne. 4) STOP 13
        s_r = SIGN(4.0, -3.0)
        if (abs(s_r - (-4.0)) > 1.0D-6) STOP 14
        s_r = SIGN(4.0, -3)
        if (abs(s_r - (-4.0)) > 1.0D-6) STOP 15
        s_r = SIGN(-4, 3.0)
        if (abs(s_r - 4.0) > 1.0D-6) STOP 16

        mx_i = MAX(-4, -3, 2, 1)
        if (mx_i .ne. 2) STOP 17
        mx_r = MAX(-4.0, -3.0, 2.0, 1.0)
        if (abs(mx_r - 2.0) > 1.0D-6) STOP 18
        mx_r = MAX(-4, -3.0, 2.0, 1)
        if (abs(mx_r - 2.0) > 1.0D-6) STOP 19
        mx_i = MAXLOC(array_nan, 1)
        if (mx_i .ne. 2) STOP 20

        mn_i = MIN(-4, -3, 2, 1)
        if (mn_i .ne. -4) STOP 21
        mn_r = MIN(-4.0, -3.0, 2.0, 1.0)
        if (abs(mn_r - (-4.0)) > 1.0D-6) STOP 22
        mn_r = MIN(-4, -3.0, 2.0, 1)
        if (abs(mn_r - (-4.0)) > 1.0D-6) STOP 23
        mn_i = MINLOC(array_nan, 1)
        if (mn_i .ne. 1) STOP 24
      END PROGRAM
