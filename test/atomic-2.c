#include <omp.h>

#define ATYPE float
ATYPE count = 0;

#define N 18
ATYPE atomic () {
#pragma omp atomic
    count++;
  return count;
}

void g()
{
  int i;
#pragma omp parallel for
  for(i = 0; i<10000000; i++) {
    atomic();
  }
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
