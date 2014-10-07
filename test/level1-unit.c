#include <omp.h>
#include <ompt.h>
#include <stdio.h>

#define OMPT_EVENT_DETAIL 0

#define OMPT_API_DECLARE(fn) fn ## _t fn

#define LOOKUP(lookup, fn) fn = (fn ## _t) lookup(#fn)

OMPT_API_DECLARE(ompt_get_parallel_id);
OMPT_API_DECLARE(ompt_set_callback);

#define N 37
long
fib(int n)
{
        if (n < 2) return (long) n;
        return fib(n-1) + fib(n-2);
}

void report_num_threads(int level)
{
#if 1
    #pragma omp critical
    {
        printf("Level %d: number of threads in the team - %d, omp_get_thread_num() = %d parallel_id = %8lx\n",
                  level, omp_get_num_threads(), omp_get_thread_num(), ompt_get_parallel_id(0));
    }
#endif
}

int main()
{
    // printf("max thread num is %d\n", omp_get_max_threads());
    omp_set_nested(1);
    omp_set_dynamic(0);
    #pragma omp parallel num_threads(3)
    {
        fib(N+3);
        report_num_threads(omp_get_level());
    }
    #pragma omp parallel num_threads(3)
    {
        fib(N+3);
        report_num_threads(omp_get_level());
    }
    return(0);
}


void ompt_event_parallel_begin_fn ( 
  ompt_task_id_t  parent_task_id,   /* id of the parent task  */ 
  ompt_frame_t *parent_task_frame,  /* frame data of parent task   */ 
  ompt_parallel_id_t parallel_id,   /* id of parallel region       */ 
  void *parallel_function)          /* outlined function           */ 
{ 
  int level = omp_get_level();
#if 0
  printf("%d region %8lx: create " 
         "ompt_event_parallel_create_fn(parent_task_id=%8lx, parent_task_frame=%8lx, "
         "parallel_id=%8lx par_fn=%p)\n",
          omp_get_thread_num(), parallel_id,
          parent_task_id, 
          parent_task_frame,
          parallel_id, 
          parallel_function);
#else
  printf("%d region %8lx: create %*s[level=%d, parent_parallel_id=%8lx]\n",
          omp_get_thread_num(), parallel_id, level+1, " ", level, ompt_get_parallel_id(0));
#endif

  fflush(stdout); 
}

void ompt_event_parallel_end_fn ( 
  ompt_task_id_t  parent_task_id,   /* id of the parent task  */ 
  ompt_frame_t *parent_task_frame,  /* frame data of parent task   */ 
  ompt_parallel_id_t parallel_id,   /* id of parallel region       */ 
  void *parallel_function)          /* outlined function           */ 
{ 
  int level = omp_get_level();
#if 0
  printf("%d region %8lx: exit   " 
         "ompt_event_parallel_exit_fn(parent_task_id=%8lx, parent_task_frame=%8lx, "
         "parallel_id=%8lx par_fn=%p)\n",
          omp_get_thread_num(), parallel_id,
          parent_task_id, 
          parent_task_frame,
          parallel_id, 
          parallel_function);
#else
  printf("%d region %8lx: end    %*s[level=%d, parent_parallel_id=%8lx]\n",
          omp_get_thread_num(), parallel_id, level+1, " ", level, ompt_get_parallel_id(0));
#endif

  fflush(stdout); 
}

#define REGISTER(EVENT) \
if (ompt_set_callback(EVENT, (ompt_callback_t) EVENT ## _fn) == 0) { \
  fprintf(stderr,"Failed to register OMPT callback %s!\n", #EVENT); return 0; \
}

int ompt_initialize(ompt_function_lookup_t lookup, const char *runtime_version, int ompt_version) {
  printf("Init: %s ver %i\n",runtime_version,ompt_version);
  LOOKUP(lookup,ompt_get_parallel_id);
  LOOKUP(lookup,ompt_set_callback);
  REGISTER(ompt_event_parallel_begin);
  REGISTER(ompt_event_parallel_end);
  return 1;
}

