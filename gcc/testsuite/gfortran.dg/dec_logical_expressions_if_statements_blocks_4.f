! { dg-do run }
! { dg-options "-fdec -Wconversion-extra" }
!
! Contributed by Francisco Redondo Marchena <francisco.marchema@codethink.co.uk>
!             and Jeff Law <law@redhat.com>
! Modified by Mark Eggleston <mark.eggleston@codethink.com>
!
       function othersub1()
        integer*4 othersub1
        othersub1 = 9
       end

       function othersub2()
        integer*4 othersub2
        othersub2 = 0
       end

       program MAIN
        integer*4 othersub1
        integer*4 othersub2
        integer a /1/
        integer b /2/        
 
        if (othersub1()) then ! { dg-warning "if it evaluates to nonzero" }
           write(*,*) "OK"
        else
           stop 1
        end if
        if (othersub2()) then ! { dg-warning "if it evaluates to nonzero" }
           stop 2
        else
           write(*,*) "OK"
        end if
        if (a-b) then ! { dg-warning "if it evaluates to nonzero" }
           write(*,*) "OK"
        else
           stop 3
        end if
        if (b-(a+1)) then ! { dg-warning "if it evaluates to nonzero" }
           stop 3
        else
           write(*,*) "OK"
        end if
       end

