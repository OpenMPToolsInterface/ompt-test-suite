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

#define NUM_THREADS 4


//*****************************************************************************
// global data
//*****************************************************************************

std::map<ompt_task_id_t, ompt_task_id_t> task_id_to_task_id_map;
std::map<ompt_task_id_t, ompt_frame_t *> task_id_to_task_frame_map;
std::set<ompt_task_id_t> task_ids;

int tasks_active = 0;



//*****************************************************************************
// private operations 
//*****************************************************************************

static void 
on_ompt_event_task_begin(ompt_task_id_t parent_task_id,    
                              ompt_frame_t *parent_task_frame,  
                              ompt_task_id_t new_task_id,       
                              void *new_task_function)
{
    task_id_to_task_id_map[new_task_id] = parent_task_id;
    task_id_to_task_frame_map[new_task_id] = parent_task_frame;
    task_ids.insert(new_task_id);
    #pragma omp atomic update
    tasks_active += 1;
}

static void
on_ompt_event_task_end(ompt_task_id_t  task_id)
{
    CHECK(task_ids.count(task_id) != 0, IMPLEMENTED_BUT_INCORRECT, \
	  "no record for task id");
    #pragma omp atomic update
    tasks_active -= 1;
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
  #pragma omp parallel num_threads(NUM_THREADS)
  {
    #pragma omp master
    {
      #pragma omp task
      {
	serialwork(0);
        #pragma omp task
	{
	  serialwork(0);
          #pragma omp task
	  {
	    serialwork(1);
	  }
	}
      }
    }
  }
  CHECK(tasks_active == 0, IMPLEMENTED_BUT_INCORRECT, \
	"unbalanced number of calls to begin and end callbacks");
  return return_code;
}
