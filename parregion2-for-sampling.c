#include <omp.h>
#include "stdio.h"

#define MAX (1LL << 22)

void g()
{
    long long j = 0;
#pragma omp parallel 
{
    long long i;
    for(i = 0; i < MAX; i++) 
#pragma omp atomic 
	j+=1;
}
    printf("j = %lld\n", j);
}

void f()
{
  g();
}

int main()
{
  f();
  return 0;
}
