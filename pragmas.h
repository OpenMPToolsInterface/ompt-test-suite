/******************************************************************************
*   OpenMp Example - Matrix Multiply - C Version
*   Demonstrates a matrix multiply using OpenMP. 
*
*   Modified from here:
*   https://computing.llnl.gov/tutorials/openMP/samples/C/omp_mm.c
*
*   For  PAPI_FP_INS, the exclusive count for the event: 
*   for (null) [OpenMP location: file:matmult.c ]
*   should be  2E+06 / Number of Threads 
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>
#include <math.h>
#include "callback.h"

int atomic () {
  int count = 0;
  int max = 2;
  #pragma omp parallel
  {
    #pragma omp atomic
    count++;
  }
  return count;
}

int barrier () {
  int count = 0;
  int max = omp_get_num_threads() == 1 ? 1 : 2;
  #pragma omp parallel num_threads(max)
  {
    sleep(omp_get_thread_num());
    #pragma omp barrier
  }
  return count;
}

#define CRITICAL_SIZE 10

int critical() {
  int i;
  int max;
  int a[CRITICAL_SIZE];

  for (i = 0; i < CRITICAL_SIZE; i++) {
    a[i] = rand();
  }

  max = a[0];
  int maxthreads = omp_get_num_threads() == 1 ? 1 : 2;
  #pragma omp parallel for num_threads(maxthreads)
  for (i = 1; i < CRITICAL_SIZE; i++) {
    if (a[i] > max) {
      #pragma omp critical
      {
        // compare a[i] and max again because max 
        // could have been changed by another thread after 
        // the comparison outside the critical section
        if (a[i] > max) {
          max = a[i];
        }
      }
    }
  }
  return max;
}

int critical_named() {
  int i;
  int max;
  int a[CRITICAL_SIZE];

  for (i = 0; i < CRITICAL_SIZE; i++) {
    a[i] = rand();
  }

  max = a[0];
  int maxthreads = omp_get_num_threads() == 1 ? 1 : 2;
  #pragma omp parallel for num_threads(maxthreads)
  for (i = 1; i < CRITICAL_SIZE; i++) {
    if (a[i] > max) {
      #pragma omp critical (accumulator)
      {
        // compare a[i] and max again because max 
        // could have been changed by another thread after 
        // the comparison outside the critical section
        if (a[i] > max) {
          max = a[i];
        }
      }
    }
  }
  return max;
}

void myread(int *data) {
  *data = 1;
}

void process(int *data) {
  (*data)++;
}

int flush() {
  int data;
  int flag = 0;

  int maxthreads = omp_get_num_threads() == 1 ? 1 : 2;
  #pragma omp parallel sections num_threads(maxthreads)
  {
    #pragma omp section
    {
      myread(&data);
      #pragma omp flush(data)
      flag = 1;
      #pragma omp flush(flag)
      // Do more work.
    }
    #pragma omp section 
    {
      while (!flag) {
        #pragma omp flush(flag)
      }
      #pragma omp flush(data)
      process(&data);
    }
  }
  return data;
}

int fortest() {
   int i, nRet = 0, nSum = 0, nStart = 0, nEnd = 10;
   int nThreads = 0, nTmp = 10;
   unsigned uTmp = 55;
   int nSumCalc = uTmp;

   if (nTmp < 0)
      nSumCalc = -nSumCalc;

   #pragma omp parallel default(none) private(i) shared(nSum, nThreads, nStart, nEnd)
   {
      #pragma omp master
      nThreads = omp_get_num_threads();

      #pragma omp for
      for (i=nStart; i<=nEnd; ++i) {
            #pragma omp atomic
            nSum += i;
      }
   }
   return nSum;
}

void foo() {
  printf("%d In foo\n", omp_get_thread_num());
}

int parallelfor() {
   int i, nStart = 0, nEnd = 10;
   #pragma omp parallel for
   for (i=nStart; i<=nEnd; ++i) {
     foo();
   }
   return 0;
}

int parallelfor_static() {
   int i, nStart = 0, nEnd = 10;
   // the ordered parameter forces the static scheduler
   #pragma omp parallel for schedule(static) ordered
   for (i=nStart; i<=nEnd; ++i) {
     foo();
   }
   return 0;
}

int parallelfor_dynamic() {
   int i, nStart = 0, nEnd = 10;
   #pragma omp parallel for schedule(dynamic)
   for (i=nStart; i<=nEnd; ++i) {
     foo();
   }
   return 0;
}

int parallelfor_runtime() {
   int i, nStart = 0, nEnd = 10;
   #pragma omp parallel for schedule(runtime)
   for (i=nStart; i<=nEnd; ++i) {
     foo();
   }
   return 0;
}

int master( ) 
{
  int a[5], i;

   #pragma omp parallel
   {
     // Perform some computation.
     #pragma omp for
     for (i = 0; i < 5; i++)
       a[i] = i * i;
        
     // Print intermediate results.
     #pragma omp master
     for (i = 0; i < 5; i++) {
       printf("a[%d] = %d\n", i, a[i]);
	   fflush(stdout);
	 }
        
     // Wait.
     #pragma omp barrier
        
     // Continue with the computation.
     #pragma omp for
     for (i = 0; i < 5; i++)
       a[i] += i;
  }
  return a[3];
}

static float a[1000], b[1000], c[1000];

void test(int first, int last) 
{
  int i;
#pragma omp for ordered schedule(dynamic) 
  for (i = first; i <= last; ++i) {
    // Do something here.
    if (i % 2) 
    {
#pragma omp ordered 
      printf("test() iteration %d, thread %d\n", i, omp_get_thread_num());
      fflush(stdout);
    }
  }
}

void test2(int iter) 
{
  #pragma omp ordered 
  printf("test2() iteration %d, thread %d\n", iter, omp_get_thread_num());
  fflush(stdout);
}

int ordered( ) 
{
  int i = 17;
#pragma omp parallel
  {
    test(1, 8);
#pragma omp for ordered schedule(dynamic,1)
    for (i = 0 ; i < 5 ; i++)
      test2(i);
  }
  return i;
}

int sections() {
  #pragma omp parallel sections
  {
    {
    printf("Hello from thread %d\n", omp_get_thread_num()); fflush(stdout);
	}
    #pragma omp section
	{
    printf("Hello from thread %d\n", omp_get_thread_num()); fflush(stdout);
	}
  }
  return 1;
}

int single( ) 
{
  int a[5], i;

   #pragma omp parallel
   {
     // Perform some computation.
     #pragma omp for
     for (i = 0; i < 5; i++)
       a[i] = i * i;
        
     // Print intermediate results.
     #pragma omp single
     for (i = 0; i < 5; i++) {
       printf("a[%d] = %d\n", i, a[i]);
       fflush(stdout);
     }
        
     // Wait.
     #pragma omp barrier
        
     // Continue with the computation.
     #pragma omp for
     for (i = 0; i < 5; i++)
       a[i] += i;
  }
  return a[3];
}

int fib(int n) {
  int x,y;
  if (n<2) return n;
  #pragma omp task untied shared(x)
  { x = fib(n-1); }
  #pragma omp task untied shared(y)
  { y = fib(n-2); }
  #pragma omp taskwait
  /*printf("%d: fib(%d)=%d\n", omp_get_thread_num(), n, x+y);*/
  fflush(stdout);

  return x+y;
}

int fibouter(int n) {
  int answer = 0;
  #pragma omp parallel shared(answer)
  {
    #pragma omp single 
    {
      #pragma omp task shared(answer) 
      {
	    answer = fib(n);
      }
    }
  }
  return answer;
}

int locktest(void) {
  omp_lock_t writelock;
  omp_init_lock(&writelock);
  int i;
  int x = 10;

#pragma omp parallel for
  for ( i = 0; i < x; i++ )
  {
    // some stuff
    omp_set_lock(&writelock);
    // one thread at a time stuff
    foo();
    omp_unset_lock(&writelock);
    // some stuff
  }
  omp_destroy_lock(&writelock);
  return 0;
}

int main (int argc, char *argv[]) 
{
  printf("Running with OMP_NUM_THREADS=%s threads\n", getenv("OMP_NUM_THREADS")); fflush(stdout);
#ifdef ATOMIC
  printf ("\n\nDoing atomic: %d\n\n", atomic()); fflush(stdout);
#endif
#ifdef BARRIER
  printf ("\n\nDoing barrier: %d\n\n", barrier()); fflush(stdout);
#endif
#ifdef MASTER
  printf ("\n\nDoing master: %d\n\n", master()); fflush(stdout);
#endif
#ifdef ORDERED
  printf ("\n\nDoing ordered: %d\n\n", ordered()); fflush(stdout);
#endif
#ifdef CRITICAL
  printf ("\n\nDoing critical: %d\n\n", critical()); fflush(stdout);
#endif
#ifdef FORTEST
  printf ("\n\nDoing fortest: %d\n\n", fortest()); fflush(stdout);
#endif
#ifdef FLUSH
  printf ("\n\nDoing flush: %d\n\n", flush()); fflush(stdout);
#endif
#ifdef SECTIONS
  printf ("\n\nDoing sections: %d\n\n", sections()); fflush(stdout);
#endif
#ifdef SINGLE
  printf ("\n\nDoing single: %d\n\n", single()); fflush(stdout);
#endif
#ifdef CRITICAL_NAMED
  printf ("\n\nDoing critical named: %d\n\n", critical_named()); fflush(stdout);
#endif
#ifdef PARALLELFOR
  printf ("\n\nDoing parallelfor: %d\n\n", parallelfor()); fflush(stdout);
#endif
#ifdef PARALLELFOR_STATIC
  printf ("\n\nDoing parallelfor_static: %d\n\n", parallelfor_static()); fflush(stdout);
#endif
#ifdef PARALLELFOR_DYNAMIC
  printf ("\n\nDoing parallelfor_dynamic: %d\n\n", parallelfor_dynamic()); fflush(stdout);
#endif
#ifdef PARALLELFOR_RUNTIME
  printf ("\n\nDoing parallelfor_runtime: %d\n\n", parallelfor_runtime()); fflush(stdout);
#endif
#ifdef LOCKTEST
  printf ("\n\nDoing locktest: %d\n\n", locktest()); fflush(stdout);
#endif
#ifdef TASKS
  printf ("\n\nDoing tasks: %d\n\n", fibouter(20)); fflush(stdout);
#endif

  printf ("Done.\n"); fflush(stdout);
  // sleep, so all threads can finish.
  sleep(1);

  return 0;
}

