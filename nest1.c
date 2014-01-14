#include <omp.h>
#include <stdio.h>
#include "callback.h"

int region=0;
#define N 37
long
fib(int n)
{
        if (n < 2) return (long) n;
        return fib(n-1) + fib(n-2);
}

void report_num_threads(int level)
{
    #pragma omp critical
    {
        printf("Level %d: number of threads in the team - %d\n",
                  level, omp_get_num_threads());
    }
 }
int main()
{
printf("max thread num is %d\n", omp_get_max_threads());
    omp_set_nested(1);
    omp_set_dynamic(0);
    region=1;
    #pragma omp parallel num_threads(2)
    {
        fib(N+3);
        report_num_threads(omp_get_level());
        #pragma omp parallel num_threads(2)
        {
                fib(N+3);
                report_num_threads(omp_get_level());
        }
    }
    return(0);
}

