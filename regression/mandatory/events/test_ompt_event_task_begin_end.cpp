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

std::map<ompt_task_id_t, ompt_task_id_t> task_id_to_task_id_map;
std::map<ompt_task_id_t, ompt_frame_t *> task_id_to_task_frame_map;
std::set<ompt_task_id_t> task_ids;

int tasks_begin = 0;
int tasks_end = 0;



//*****************************************************************************
// private operations 
//*****************************************************************************

static void 
on_ompt_event_task_begin(ompt_task_id_t parent_task_id,    
                              ompt_frame_t *parent_task_frame,  
                              ompt_task_id_t new_task_id,       
                              void *new_task_function)
{
#if DEBUG
    pthread_mutex_lock(&thread_mutex);
    printf("task_begin %lld (parent %lld)\n", new_task_id, parent_task_id);
    pthread_mutex_unlock(&thread_mutex);
#endif

    task_id_to_task_id_map[new_task_id] = parent_task_id;
    task_id_to_task_frame_map[new_task_id] = parent_task_frame;
    task_ids.insert(new_task_id);
    #pragma omp atomic update
    tasks_begin += 1;
}

static void
on_ompt_event_task_end(ompt_task_id_t  task_id)
{
#if DEBUG
    pthread_mutex_lock(&thread_mutex);
    printf("task_end %lld\n", task_id);
    pthread_mutex_unlock(&thread_mutex);
#endif

    CHECK(task_ids.count(task_id) != 0, IMPLEMENTED_BUT_INCORRECT, \
	  "no record for task id");
    #pragma omp atomic update
    tasks_end += 1;
}



//*****************************************************************************
// interface operations 
//*****************************************************************************

void 
init_test(ompt_function_lookup_t lookup)
{
  if (!register_callback(ompt_event_task_begin, 
			 (ompt_callback_t) on_ompt_event_task_begin)) {
    CHECK(false, FATAL, "failed to register ompt_event_task_begin");
  }
  if (!register_callback(ompt_event_task_end, 
			 (ompt_callback_t) on_ompt_event_task_end)) {
    CHECK(false, FATAL, "failed to register ompt_event_task_begin");
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
