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
// global variables
//*****************************************************************************

static std::map<ompt_parallel_id_t, ompt_task_id_t> parallel_id_to_task_id;
static std::set<ompt_task_id_t> task_ids;

int regions_active = 0;

ompt_task_id_t serial_task_id;
ompt_frame_t * serial_task_frame;

bool test_enclosing_context = false;



//*****************************************************************************
// private operations 
//*****************************************************************************

static void
on_ompt_event_parallel_begin
(ompt_task_id_t parent_task_id,    /* id of parent task            */
 ompt_frame_t *parent_task_frame,  /* frame data of parent task    */
 ompt_parallel_id_t parallel_id,   /* id of parallel region        */
 uint32_t requested_team_size,     /* number of pregions in team   */
 void *parallel_function           /* pointer to outlined function */
)
{
  pthread_mutex_lock(&thread_mutex);
#if DEBUG
  printf("begin parallel: parallel_id = %" PRIu64 ", "
	 "parent_task_frame %p, parent_task_id = %" PRIu64 "\n", 
	 parallel_id, parent_task_frame, parent_task_id);
#endif
  parallel_id_to_task_id[parallel_id] = parent_task_id;
  task_ids.insert(parent_task_id);
  regions_active += 1;
  pthread_mutex_unlock(&thread_mutex);
}

static void
on_ompt_event_parallel_end
(ompt_parallel_id_t parallel_id,    /* id of parallel region       */
 ompt_task_id_t task_id             /* id of task                  */
 )
{
  pthread_mutex_lock(&thread_mutex);
#if DEBUG
  printf("end parallel: parallel_id = %" PRIu64 ", task_id = %" PRIu64 "\n", 
	 parallel_id, task_id);
#endif
  CHECK(parallel_id_to_task_id.count(parallel_id) != 0,	  \
	IMPLEMENTED_BUT_INCORRECT,			  \
	"no record found for parallel id");
  
  CHECK(task_ids.count(task_id) != 0, IMPLEMENTED_BUT_INCORRECT,	\
	"end for task_id %" PRIu64 " with no matching begin", task_id);
  
  parallel_id_to_task_id.erase(parallel_id);
  task_ids.erase(task_id);
  regions_active -= 1;
  pthread_mutex_unlock(&thread_mutex);
  
  if (test_enclosing_context) {
    CHECK(ompt_get_task_id(0) == serial_task_id,			\
	  IMPLEMENTED_BUT_INCORRECT,					\
	  "parallel end callback for region %" PRIu64 " "		\
	  "doesn't execute in parent's context", parallel_id);
    CHECK(ompt_get_task_frame(0) == serial_task_frame,			\
	  IMPLEMENTED_BUT_INCORRECT,					\
	  "parallel end callback for region %" PRIu64 " "
	  "doesn't execute in parent's context", parallel_id);
  }
}



//*****************************************************************************
// interface operations 
//*****************************************************************************

void 
init_test(ompt_function_lookup_t lookup)
{
  if (!register_callback(ompt_event_parallel_begin, 
			 (ompt_callback_t) on_ompt_event_parallel_begin)) {
    CHECK(FALSE, FATAL, "failed to register ompt_event_parallel_begin");
  }
  if (!register_callback(ompt_event_parallel_end, 
			 (ompt_callback_t) on_ompt_event_parallel_end)) {
    CHECK(FALSE, FATAL, "failed to register ompt_event_parallel_end");
  }
}


int
regression_test(int argc, char** argv)
{
  serial_task_id = ompt_get_task_id(0);
  serial_task_frame = ompt_get_task_frame(0);
  
#if DEBUG
  printf("serial_task_id = %" PRIu64 ", serial_task_frame = %p\n", 
	 serial_task_id, serial_task_frame);
#endif

  // single-level parallel region
  test_enclosing_context = true;
  #pragma omp parallel num_threads(NUM_THREADS)
  {
    serialwork(0);
  }
  test_enclosing_context = false;
  
  // two-level nested parallel region (one nested instance)
  test_enclosing_context = true;
  #pragma omp parallel num_threads(NUM_THREADS) 
  {
    #pragma omp barrier 
    test_enclosing_context = false;
    #pragma omp master 
    {
      #pragma omp parallel num_threads(NUM_THREADS)
      {
	serialwork(0);
      }
    }
    #pragma omp barrier 
    test_enclosing_context = true;
  }
  test_enclosing_context = false;
  
  // two-level nested parallel region (multiple nested instances)
  test_enclosing_context = true;
  #pragma omp parallel num_threads(NUM_THREADS) 
  {
    #pragma omp barrier 
    test_enclosing_context = false;
    #pragma omp parallel num_threads(NUM_THREADS)
    {
      serialwork(0);
    }
    #pragma omp barrier 
    test_enclosing_context = true;
  }
  test_enclosing_context = false;
  
  //  three-level nested parallel region (multiple levels of single instance)
  test_enclosing_context = true;
  #pragma omp parallel num_threads(NUM_THREADS) 
  {
    #pragma omp barrier 
    test_enclosing_context = false;
    #pragma omp master 
    {
      #pragma omp parallel num_threads(NUM_THREADS)
      {
        #pragma omp master 
	{
          #pragma omp parallel num_threads(NUM_THREADS)
	  {
	    serialwork(0);
	  }
	}
      }
    }
    #pragma omp barrier 
    test_enclosing_context = true;
  }
  test_enclosing_context = false;
  
  // three-level nested parallel region (multiple levels of multiple instances)
  test_enclosing_context = true;
  #pragma omp parallel num_threads(NUM_THREADS) 
  {
    #pragma omp barrier 
    test_enclosing_context = false;
    #pragma omp parallel num_threads(NUM_THREADS)
    {
      #pragma omp parallel num_threads(NUM_THREADS)
      {
	serialwork(0);
      }
    }
    #pragma omp barrier 
    test_enclosing_context = true;
  }
  test_enclosing_context = false;
  
  parallel_id_to_task_id.clear();
  
  CHECK(regions_active == 0, IMPLEMENTED_BUT_INCORRECT, 
	"number of calls to parallel begin differs from the number of calls to end");

  return return_code;
}
