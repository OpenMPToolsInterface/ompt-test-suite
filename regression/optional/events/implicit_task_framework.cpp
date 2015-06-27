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
// global data
//*****************************************************************************

std::set<ompt_task_id_t> active_task_ids;
std::set<ompt_task_id_t> all_task_ids;

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

    std::set<ompt_task_id_t>::iterator iter = all_task_ids.find(task_id);

    active_task_ids.insert(task_id);

    if (iter == all_task_ids.end()) {
        all_task_ids.insert(task_id);
        #pragma omp atomic update
        tasks_begin += 1;
    } else {
        CHECK(FALSE, \
            IMPLEMENTED_BUT_INCORRECT, \
            "duplicate implicit task id %lld", *iter);
    }

    pthread_mutex_unlock(&thread_mutex);
}

static void
on_ompt_event_implicit_task_end(ompt_parallel_id_t parallel_id, 
                                ompt_task_id_t task_id)
{
    pthread_mutex_lock(&thread_mutex);

#if DEBUG
    printf("implicit task_end   %lld (region %lld)\n", task_id, parallel_id);
#endif

    if (active_task_ids.erase(task_id) != 0) {
        #pragma omp atomic update
        tasks_end += 1;
    } else {
        CHECK(FALSE, IMPLEMENTED_BUT_INCORRECT, \
	      "no record for task id %lld", task_id);
    }

    pthread_mutex_unlock(&thread_mutex);
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
  omp_set_nested(NESTED_VALUE);

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

  int outer_threads, inner_threads, i;
  for (outer_threads = 1; outer_threads <= NUM_THREADS; outer_threads++) {
    for (inner_threads = 1; inner_threads <= NUM_THREADS; inner_threads++) {
#if DEBUG
      printf("outer_threads=%d, inner_threads=%d\n", outer_threads, inner_threads);
#endif

      tasks_begin = tasks_end = 0;

      #pragma omp parallel num_threads(outer_threads)
      {
        #pragma omp parallel num_threads(inner_threads)
        {
          serialwork(0);
        }
      }

      // special case for gcc; use schedule(runtime) to force library call
      #pragma omp parallel num_threads(outer_threads)
      {
        #pragma omp parallel for schedule(runtime) num_threads(inner_threads)
        for (i = 0; i < NUM_THREADS; i++) {
          serialwork(0);
        }
      }

      // special case for gcc
      #pragma omp parallel num_threads(outer_threads)
      {
        #pragma omp parallel sections num_threads(inner_threads)
        {
          #pragma omp section
          {
            serialwork(0);
          }
          #pragma omp section
          {
            serialwork(0);
          }
        }
      }

      CHECK(tasks_begin == tasks_end, IMPLEMENTED_BUT_INCORRECT, \
	    "wrong number of callbacks for implicit tasks begin/end: " \
	        "outer_threads=%d, inner_threads=%d, " \
            "implicit task begin callbacks = %d, " \
            "implicit task end callbacks = %d", \
            outer_threads, inner_threads, tasks_begin, tasks_end);
    }
  }

  return return_code;
}
