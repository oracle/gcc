! { dg-do run }
! { dg-options -fdec }
!
! Test case for the default field widths enabled by the -fdec-format-defaults flag.
!
! This feature is not part of any Fortran standard, but it is supported by the
! Oracle Fortran compiler and others.

    character(50) :: buffer
    character(1) :: colon

    integer*2 :: integer_2
    integer*4 :: integer_4
    integer*8 :: integer_8

    write(buffer, '(A, I, A)') ':',12340,':'
    print *,buffer
    if (buffer.ne.":       12340:") call abort

    read(buffer, '(A1, I, A1)') colon, integer_4, colon
    if (integer_4.ne.12340) call abort

    integer_2 = -99
    write(buffer, '(A, I, A)') ':',integer_2,':'
    print *,buffer
    if (buffer.ne.":    -99:") call abort

    integer_8 = -11112222
    write(buffer, '(A, I, A)') ':',integer_8,':'
    print *,buffer
    if (buffer.ne.":              -11112222:") call abort

! If the width is 7 and there are 7 leading zeroes, the result should be zero.
    integer_2 = 789
    buffer = '0000000789'
    read(buffer, '(I)') integer_2
    if (integer_2.ne.0) call abort
end
