//*****************************************************************************
// system includes
//*****************************************************************************

#include <map>
#include <set>



//*****************************************************************************
// OpenMP runtime includes
//*****************************************************************************

#include <omp.h>



//*****************************************************************************
// regression harness includes
//*****************************************************************************

#include <ompt-regression.h>
#include <ompt-initialize.h>



//*****************************************************************************
// macros
//*****************************************************************************

#define DEBUG 0
#define NUM_THREADS 4


//*****************************************************************************
// global data
//*****************************************************************************

std::set<ompt_task_id_t> task_ids;

int tasks_begin = 0;
int tasks_end = 0;



//*****************************************************************************
// private operations 
//*****************************************************************************

static void 
on_ompt_event_implicit_task_begin(ompt_parallel_id_t parallel_id, 
                                  ompt_task_id_t task_id)
{
    pthread_mutex_lock(&thread_mutex);

#if DEBUG
    printf("implicit task_begin %lld (region %lld)\n", task_id, parallel_id);
#endif

    task_ids.insert(task_id);
    pthread_mutex_unlock(&thread_mutex);
    #pragma omp atomic update
    tasks_begin += 1;
}

static void
on_ompt_event_implicit_task_end(ompt_parallel_id_t parallel_id, 
                                ompt_task_id_t task_id)
{
    pthread_mutex_lock(&thread_mutex);

#if DEBUG
    printf("implicit task_end   %lld (region %lld)\n", task_id, parallel_id);
#endif

    CHECK(task_ids.count(task_id) != 0, IMPLEMENTED_BUT_INCORRECT, \
	  "no record for task id %lld", task_id);

    pthread_mutex_unlock(&thread_mutex);

    #pragma omp atomic update
    tasks_end += 1;
}



//*****************************************************************************
// interface operations 
//*****************************************************************************

void 
init_test(ompt_function_lookup_t lookup)
{
  if (!register_callback(ompt_event_implicit_task_begin, 
			 (ompt_callback_t) on_ompt_event_implicit_task_begin)) {
    CHECK(false, NOT_IMPLEMENTED, \
          "failed to register ompt_event_implicit_task_begin");
  }
  if (!register_callback(ompt_event_implicit_task_end, 
			 (ompt_callback_t) on_ompt_event_implicit_task_end)) {
    CHECK(false, NOT_IMPLEMENTED, \
          "failed to register ompt_event_implicit_task_begin");
  }
}


int
regression_test(int argc, char** argv)
{
  int nthreads;
  #pragma omp parallel num_threads(NUM_THREADS)
  {
    #pragma omp master 
    {
      nthreads = omp_get_num_threads();
    }
    serialwork(0);
  }
  CHECK(nthreads == tasks_begin && tasks_begin == tasks_end, IMPLEMENTED_BUT_INCORRECT, \
	"wrong number of callbacks for implicit tasks begin/end: " \
        "threads in region = %d, implicit task begin callbacks = %d, " \
        "implicit task end callbacks = %d", nthreads, tasks_begin, tasks_end);

  return return_code;
}
