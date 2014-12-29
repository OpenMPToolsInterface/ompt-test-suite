#include <omp.h>
#include <common.h>
#include <iostream>
#include <sstream>      
#include <set>
#include <map>
#define NUM_THREADS 4

using namespace std;
typedef map<ompt_thread_id_t, int> thread_id_map_t;
thread_id_map_t thread_id_map;

void 
on_ompt_event_thread_begin(ompt_thread_type_t thread_type, ompt_thread_id_t thread_id){
    thread_id_map[thread_id] = thread_type;
}

void 
on_ompt_event_thread_end(ompt_thread_type_t thread_type, ompt_thread_id_t thread_id){

  /* for the end of the thread, only worker threads is invoked, is ompt_thread_other possible here? */
  // CHECK(thread_type == ompt_thread_worker, IMPLEMENTED_BUT_INCORRECT, "only worker threads is invoked");

  /* the thread id should be the same as the thread id in the thread_begin */
  CHECK(thread_id_map.count(thread_id)>0, IMPLEMENTED_BUT_INCORRECT, "thread is should be seen before");
}

void 
init_test(ompt_function_lookup_t lookup)
{
    if (!register_callback(ompt_event_thread_begin, (ompt_callback_t) on_ompt_event_thread_begin)) {
        CHECK(FALSE, NOT_IMPLEMENTED, "failed to register ompt_event_thread_begin");
        exit(NOT_IMPLEMENTED);
    }
    if (!register_callback(ompt_event_thread_end, (ompt_callback_t) on_ompt_event_thread_end)) {
        CHECK(FALSE, NOT_IMPLEMENTED, "failed to register ompt_event_thread_end");
        exit(NOT_IMPLEMENTED);
    }
}

int
main(int argc, char** argv)
{
    register_segv_handler(argv);
    warmup();
    #pragma omp parallel num_threads(NUM_THREADS)
    {
        #pragma omp parallel num_threads(NUM_THREADS)
        {
            #pragma omp parallel num_threads(NUM_THREADS)
            {
                serialwork(0);
            }
        }
    }
    CHECK(thread_id_map.size() == (NUM_THREADS), IMPLEMENTED_BUT_INCORRECT, "wrong number of calls to ompt_event_threadbegins");
    return global_error_code;
}
