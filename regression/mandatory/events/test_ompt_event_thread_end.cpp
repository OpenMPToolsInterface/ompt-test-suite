#include <omp.h>
#include <common.h>
#include <iostream>
#include <sstream>      
#include <set>
#include <map>
using namespace std;
typedef map<ompt_thread_id_t, int> thread_id_map_t;
thread_id_map_t thread_id_map;

void 
on_ompt_event_thread_begin(ompt_thread_type_t thread_type, ompt_thread_id_t thread_id){


  if (omp_get_thread_num() == 0) {
    // master thread must be initial thread
    ASSERT(thread_type == ompt_thread_initial, IMPLEMENTED_BUT_INCORRECT, "master thread must be initial thread");
  } else {
    // if it is not master, it must be worker thread
    ASSERT(thread_type == ompt_thread_worker, IMPLEMENTED_BUT_INCORRECT, "if it is not master, it must be a worker thread");

    // the thread id must be unique
    ASSERT(thread_id_map.count(thread_id)==0, IMPLEMENTED_BUT_INCORRECT, "thread id must be unique");
    thread_id_map[thread_id] = thread_type;
  }
}

void 
on_ompt_event_thread_end(ompt_thread_type_t thread_type, ompt_thread_id_t thread_id){

  // for the end of the thread, only worker threads is invoked
  ASSERT(thread_type == ompt_thread_worker, IMPLEMENTED_BUT_INCORRECT, "only worker threads is invoked");

  // the thread id should be the same as the thread id in the thread_begin
  ASSERT(thread_id_map.count(thread_id)>0, IMPLEMENTED_BUT_INCORRECT, "thread is should be seen before");
}

void 
init_test(ompt_function_lookup_t lookup)
{
    if (!register_callback(ompt_event_thread_begin, (ompt_callback_t) on_ompt_event_thread_begin)) {
        cout << "Failed to register ompt_event_thread_begin" << endl;
        exit(NOT_IMPLEMENTED);
    }
    if (!register_callback(ompt_event_thread_end, (ompt_callback_t) on_ompt_event_thread_end)) {
        cout << "Failed to register ompt_event_thread_end" << endl;
        exit(NOT_IMPLEMENTED);
    }
}

int
main(int argc, char** argv)
{
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
    ASSERT(thread_id_map.size() == (NUM_THREADS-1), IMPLEMENTED_BUT_INCORRECT, "Wrong number of calls to ompt_event_threadbegins");
    return CORRECT;
}
