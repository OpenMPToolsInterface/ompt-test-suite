#include <omp.h>
#include <common.h>
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

int active_workers = 0;
int total_workers = 0;

void 
on_ompt_event_thread_begin(ompt_thread_type_t thread_type, ompt_thread_id_t thread_id){
  pthread_mutex_lock(&mutex);
  if (thread_type == ompt_thread_worker) {
     active_workers++;
     total_workers++;
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
    active_workers--;
  }
  pthread_mutex_unlock(&mutex);
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

    CHECK(total_workers > 0, IMPLEMENTED_BUT_INCORRECT, "no worker threads created");

    CHECK(active_workers == 0, IMPLEMENTED_BUT_INCORRECT, \
          "mismatch between number of calls to ompt_event_thread_begin/end");

    return global_error_code;
}
