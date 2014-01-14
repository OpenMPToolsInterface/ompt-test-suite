#include <stdio.h>
#include "omp.h"
#include <ompt.h>

// extern int omp_get_num_threads(void);

#include "callback.h"
#include "dump.h"

main()
{
	omp_set_nested(1);
#pragma omp parallel 
{
	int rank = omp_get_thread_num();
        int nthreads = omp_get_num_threads();
	printf("hello world from %d (level 1) %d threads\n", rank, nthreads);
	dump_frames(rank, 1);
#pragma omp parallel 
{
        int nthreads = omp_get_num_threads();
	printf("hello world from %d (level 2) %d threads\n", rank, nthreads);
	dump_frames(rank, 2);
#pragma omp parallel 
{
        int nthreads = omp_get_num_threads();
	printf("hello world from %d (level 3) %d threads\n", rank, nthreads);
	dump_frames(rank, 3);
}
}
}
}
