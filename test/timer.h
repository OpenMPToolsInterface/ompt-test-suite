#include <time.h>

struct timespec start_time; 

void timer_start()
{
    clock_gettime(CLOCK_REALTIME, &start_time);
}

double timer_elapsed()
{
    struct timespec end_time;
    clock_gettime(CLOCK_REALTIME, &end_time);
    return (((double) (end_time.tv_sec - start_time.tv_sec)) +  /* sec */
	    ((double)(end_time.tv_nsec - start_time.tv_nsec))/1000000000); /*nanosec */
}
