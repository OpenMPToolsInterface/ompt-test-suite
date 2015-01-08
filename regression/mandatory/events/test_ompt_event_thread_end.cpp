#include <omp.h>
#include <common.h>
#include <unistd.h>
#include <iostream>
#include <sstream>      
#include <set>
#include <map>

#include <pthread.h>

#define NUM_THREADS 4

using namespace std;
typedef map<ompt_thread_id_t, int> thread_id_map_t;
thread_id_map_t thread_id_map;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int worker_begin = 0;
int worker_end = 0;


void 
on_ompt_event_thread_begin(ompt_thread_type_t thread_type, ompt_thread_id_t thread_id){
  pthread_mutex_lock(&mutex);
  if (thread_type == ompt_thread_worker) {
     worker_begin++;
  }
  thread_id_map[thread_id] = thread_type;
  pthread_mutex_unlock(&mutex);
}


void 
on_ompt_event_thread_end(ompt_thread_type_t thread_type, ompt_thread_id_t thread_id){

  /* for the end of the thread, only worker threads is invoked, is ompt_thread_other possible here? */
  // CHECK(thread_type == ompt_thread_worker, IMPLEMENTED_BUT_INCORRECT, "only worker threads is invoked");

  pthread_mutex_lock(&mutex);
  // the thread id should be the same as the thread id in the thread_begin */
  CHECK(thread_id_map.count(thread_id)>0, IMPLEMENTED_BUT_INCORRECT, \
        "thread should have been entered in map during ompt_event_thread_begin");
  if (thread_type == ompt_thread_worker) {
    worker_end++;
  }
  pthread_mutex_unlock(&mutex);
}


static void thread_end_check() 
{
    CHECK(worker_begin == worker_end, IMPLEMENTED_BUT_INCORRECT, \
          "mismatch between number of calls to ompt_event_thread_begin/end: " \
	  "%d workers begin, %d workers end", worker_begin, worker_end);

    // force an immediate exit with the proper exit status. this will override
    // any status passed to exit() in main
    _exit(global_error_code);
}


void 
init_test(ompt_function_lookup_t lookup)
{
    if (!register_callback(ompt_event_thread_begin, (ompt_callback_t) on_ompt_event_thread_begin)) {
        CHECK(false, FATAL, "failed to register ompt_event_thread_begin");
    }
    if (!register_callback(ompt_event_thread_end, (ompt_callback_t) on_ompt_event_thread_end)) {
        CHECK(false, FATAL, "failed to register ompt_event_thread_end");
    }
    if (!register_callback(ompt_event_runtime_shutdown, (ompt_callback_t) thread_end_check)) {
        CHECK(false, FATAL, "failed to register ompt_event_shutdown");
    }
}


int
main(int argc, char** argv)
{
    register_segv_handler(argv);
    warmup();

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
    return SHUTDOWN_FAILED_TO_PREEMPT_EXIT;
}
