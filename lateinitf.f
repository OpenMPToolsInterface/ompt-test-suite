        subroutine foo()
        integer rank
!omp parallel
                rank = omp_get_thread_num()
                print *, "hello world from ", rank
!omp parallel end
        end subroutine foo

        program main
        call foo
        end program main
