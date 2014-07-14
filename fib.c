#include <omp.h>
#include <stdio.h>

#include "fib.h"

void
usage()
{
    printf("usage: fib n, n >= 0\n");
    exit(-1);
}


int main(int argc, char **argv)
{
  int n;

  if (argc < 2) usage(); 
  n = atoi(argv[1]);
  if (n  < 0) usage();

  printf("fib(%d) = %d\n", n, fib(n));

  return 0;
}
