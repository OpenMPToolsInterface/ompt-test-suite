#include <stdio.h>
#include "omp.h"
#include "callback.h"
main()
{
#pragma omp parallel
{
	int rank = omp_get_thread_num();
	printf("hello world from %d\n", rank);
}
}
