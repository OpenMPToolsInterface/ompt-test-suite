        program atest
        complex c
!$omp  parallel do
        do i = 0, 10000000
!$omp  atomic 
        c = c * 1.0
        enddo
        end
