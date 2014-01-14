#include <omp.h>

#define MAX (1LL << 30)

void g()
{
#pragma omp parallel 
{
    int j;
    if (omp_get_thread_num() == 0) {
      for(j = 0; j < MAX; j++);
    }
    else {
      for(j = 0; j < MAX; j++);
    }
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
