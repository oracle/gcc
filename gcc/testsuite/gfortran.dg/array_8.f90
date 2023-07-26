! { dg-do compile }
! { dg-options "-fdec -fno-dec-add-missing-indexes" }!
! Checks that under-specified arrays (referencing arrays with fewer
! dimensions than the array spec) generates a warning.
!
! Contributed by Jim MacArthur <jim.macarthur@codethink.co.uk>
! Updated by Mark Eggleston <mark.eggleston@codethink.co.uk>
!

program under_specified_array
    integer chessboard(8,8)
    integer chessboard3d(8,8,3:5)
    chessboard(3,1) = 5
    chessboard(3,2) = 55
    chessboard3d(4,1,3) = 6
    chessboard3d(4,1,4) = 66
    chessboard3d(4,4,3) = 7
    chessboard3d(4,4,4) = 77
  
    if (chessboard(3).ne.5) stop 1  ! { dg-error "Rank mismatch" }
    if (chessboard3d(4).ne.6) stop 2  ! { dg-error "Rank mismatch" }
    if (chessboard3d(4,4).ne.7) stop 3  ! { dg-error "Rank mismatch" }
end program
