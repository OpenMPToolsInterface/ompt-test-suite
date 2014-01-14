#include <stdio.h>
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
#pragma omp parallel 
{
	printf("hello world from %d (level 3)\n", rank);
	dump_frames(rank, 3);
#pragma omp parallel 
{
	printf("hello world from %d (level 4)\n", rank);
	dump_frames(rank, 4);
#pragma omp parallel 
{
	printf("hello world from %d (level 5)\n", rank);
	dump_frames(rank, 5);

}
	printf("exiting %d (level 4)\n", rank);
	dump_frames(rank, 4);
}
}
}
}
}
