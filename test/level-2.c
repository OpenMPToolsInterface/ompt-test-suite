#include <stdio.h>
#include "omp.h"
#include <ompt.h>

#include "callback.h"
#include "dump.h"

main()
{
#pragma omp parallel
{
	int rank = omp_get_thread_num();
	printf("hello world from %d (level 1)\n", rank);
	dump_frames(rank, 1);
#pragma omp parallel 
{
	printf("hello world from %d (level 2)\n", rank);
	dump_frames(rank, 2);
}
}
}
