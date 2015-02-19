//*****************************************************************************
// system includes
//*****************************************************************************

#include <map>
#include <unistd.h>



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

std::map<ompt_thread_id_t, int> thread_id_map;

int worker_begin = 0;
int worker_end = 0;



//*****************************************************************************
// private operations
//*****************************************************************************

static void 
on_ompt_event_thread_begin(ompt_thread_type_t thread_type, 
			   ompt_thread_id_t thread_id){
  pthread_mutex_lock(&thread_mutex);
  if (thread_type == ompt_thread_worker) {
     worker_begin++;
  }
  thread_id_map[thread_id] = thread_type;
  pthread_mutex_unlock(&thread_mutex);
}


static void 
on_ompt_event_thread_end(ompt_thread_type_t thread_type, 
			 ompt_thread_id_t thread_id){

  // for the end of the thread, only worker threads are checked 
  // are initial or other threads required? 

  pthread_mutex_lock(&thread_mutex);
  // the thread id should be the same as the thread id in the thread_begin */
  CHECK(thread_id_map.count(thread_id)>0, IMPLEMENTED_BUT_INCORRECT, \
        "thread should have been entered in map during ompt_event_thread_begin");
  if (thread_type == ompt_thread_worker) {
    worker_end++;
  }
  pthread_mutex_unlock(&thread_mutex);
}


static void 
thread_end_check() 
{
  CHECK(worker_begin == worker_end, IMPLEMENTED_BUT_INCORRECT,		\
	"mismatch between number of calls to ompt_event_thread_begin/end: " \
	"%d workers begin, %d workers end", worker_begin, worker_end);
  
  // force an immediate exit with the proper exit status. this will override
  // any status passed to exit() in main
  _exit(return_code);
}



//*****************************************************************************
// interface operations
//*****************************************************************************

void 
init_test(ompt_function_lookup_t lookup)
{
  if (!register_callback(ompt_event_thread_begin, 
			 (ompt_callback_t) on_ompt_event_thread_begin)) {
    CHECK(false, NOT_IMPLEMENTED, "failed to register ompt_event_thread_begin");
  }
  if (!register_callback(ompt_event_thread_end, 
			 (ompt_callback_t) on_ompt_event_thread_end)) {
    CHECK(false, NOT_IMPLEMENTED, "failed to register ompt_event_thread_end");
  }
  if (!register_callback(ompt_event_runtime_shutdown, 
			 (ompt_callback_t) thread_end_check)) {
    CHECK(false, NOT_IMPLEMENTED, "failed to register ompt_event_shutdown");
  }
}


int
regression_test(int argc, char** argv)
{
  #pragma omp parallel 
  {
    serialwork(0);
  }
  
  CHECK(worker_begin > 0, IMPLEMENTED_BUT_INCORRECT, "no worker threads created");
  
  //-------------------------------------------------------------------------------
  // below, initiate an erroneous exit. the only way the exit will succeed is if
  // (1) the on_event_runtime_shutdown handler gets invoked
  // (2) the test in the shutdown handler succeeds, finding that all worker threads
  //     have terminated.
  // (3) the shutdown handler forces an immediate exit with the proper exit status
  //-------------------------------------------------------------------------------
  return OMPT_SHUTDOWN_FAILED_TO_PREEMPT_EXIT;
}
