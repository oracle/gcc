! { dg-do run }
! { dg-options "-fdec" }
!
! Test promotion between integers and reals in intrinsic operations.
! These operations are: mod, modulo, dim, sign, min, max, minloc and
! maxloc.
!
! Contributed by Francisco Redondo Marchena <francisco.marchena@codethink.co.uk>
!             and Jeff Law <law@redhat.com>
! Modified by Mark Eggleston <mark.eggleston@codethink.com>
!
      PROGRAM promotion_int_real
        REAL l/0.0/
        INTEGER a_i/4/
        INTEGER*4 a2_i/4/
        INTEGER b_i/3/
        INTEGER*8 b2_i/3/
        INTEGER x_i/2/
        INTEGER y_i/1/
        REAL a_r/4.0/
        REAL*4 a2_r/4.0/
        REAL b_r/3.0/
        REAL*8 b2_r/3.0/
        REAL x_r/2.0/
        REAL y_r/1.0/

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

        ! array_nan 4th position value is NAN
        array_nan(4) = 0/l

        m_i = MOD(a_i, b_i)
        if (m_i .ne. 1) STOP 1
        m_i = MOD(a2_i, b2_i)
        if (m_i .ne. 1) STOP 2
        m_r = MOD(a_r, b_r)
        if (abs(m_r - 1.0) > 1.0D-6) STOP 3
        m_r = MOD(a2_r, b2_r)
        if (abs(m_r - 1.0) > 1.0D-6) STOP 4
        m_r = MOD(a_i, b_r)
        if (abs(m_r - 1.0) > 1.0D-6) STOP 5
        m_r = MOD(a_r, b_i)
        if (abs(m_r - 1.0) > 1.0D-6) STOP 6

        md_i = MODULO(a_i, b_i)
        if (md_i .ne. 1) STOP 7
        md_i = MODULO(a2_i, b2_i)
        if (md_i .ne. 1) STOP 8
        md_r = MODULO(a_r, b_r)
        if (abs(md_r - 1.0) > 1.0D-6) STOP 9
        md_r = MODULO(a2_r, b2_r)
        if (abs(md_r - 1.0) > 1.0D-6) STOP 10
        md_r = MODULO(a_i, b_r)
        if (abs(md_r - 1.0) > 1.0D-6) STOP 11
        md_r = MODULO(a_r, b_i)
        if (abs(md_r - 1.0) > 1.0D-6) STOP 12

        d_i = DIM(a_i, b_i)
        if (d_i .ne. 1) STOP 13
        d_i = DIM(a2_i, b2_i)
        if (d_i .ne. 1) STOP 14
        d_r = DIM(a_r, b_r)
        if (abs(d_r - 1.0) > 1.0D-6) STOP 15
        d_r = DIM(a2_r, b2_r)
        if (abs(d_r - 1.0) > 1.0D-6) STOP 16
        d_r = DIM(a_r, b_i)
        if (abs(d_r - 1.0) > 1.0D-6) STOP 17
        d_r = DIM(b_i, a_r)
        if (abs(d_r) > 1.0D-6) STOP 18

        s_i = SIGN(-a_i, b_i)
        if (s_i .ne. 4) STOP 19
        s_i = SIGN(-a2_i, b2_i)
        if (s_i .ne. 4) STOP 20
        s_r = SIGN(a_r, -b_r)
        if (abs(s_r - (-a_r)) > 1.0D-6) STOP 21
        s_r = SIGN(a2_r, -b2_r)
        if (abs(s_r - (-a2_r)) > 1.0D-6) STOP 22
        s_r = SIGN(a_r, -b_i)
        if (abs(s_r - (-a_r)) > 1.0D-6) STOP 23
        s_r = SIGN(-a_i, b_r)
        if (abs(s_r - a_r) > 1.0D-6) STOP 24

        mx_i = MAX(-a_i, -b_i, x_i, y_i)
        if (mx_i .ne. x_i) STOP 25
        mx_i = MAX(-a2_i, -b2_i, x_i, y_i)
        if (mx_i .ne. x_i) STOP 26
        mx_r = MAX(-a_r, -b_r, x_r, y_r)
        if (abs(mx_r - x_r) > 1.0D-6) STOP 27
        mx_r = MAX(-a_r, -b_r, x_r, y_r)
        if (abs(mx_r - x_r) > 1.0D-6) STOP 28
        mx_r = MAX(-a_i, -b_r, x_r, y_i)
        if (abs(mx_r - x_r) > 1.0D-6) STOP 29
        mx_i = MAXLOC(array_nan, 1)
        if (mx_i .ne. 2) STOP 30

        mn_i = MIN(-a_i, -b_i, x_i, y_i)
        if (mn_i .ne. -a_i) STOP 31
        mn_i = MIN(-a2_i, -b2_i, x_i, y_i)
        if (mn_i .ne. -a2_i) STOP 32
        mn_r = MIN(-a_r, -b_r, x_r, y_r)
        if (abs(mn_r - (-a_r)) > 1.0D-6) STOP 33
        mn_r = MIN(-a2_r, -b2_r, x_r, y_r)
        if (abs(mn_r - (-a2_r)) > 1.0D-6) STOP 34
        mn_r = MIN(-a_i, -b_r, x_r, y_i)
        if (abs(mn_r - (-a_r)) > 1.0D-6) STOP 35
        mn_i = MINLOC(array_nan, 1)
        if (mn_i .ne. 1) STOP 36
      END PROGRAM
