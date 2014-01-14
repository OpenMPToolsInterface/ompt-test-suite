#include <stdio.h>
#include <ompt.h>

#include "callback.h"
#include "dump.h"

main()
{
#pragma omp parallel
{
#pragma omp task
{
	int rank = omp_get_thread_num();
	printf("hello world from %d (level 1)\n", rank);
//	dump_frames(rank, 1);
	dump_tasks(rank, 1);
}
}
}
