#include <omp.h>
#include <common.h>
#include <iostream>
#include <sstream>      
#include <map>

using namespace std;


typedef map<ompt_task_id_t, ompt_thread_type_t> threadIdMap_t;
static threadIdMap_t thread_id_map;

void
on_ompt_event_thread_begin(ompt_thread_type_t thread_type, ompt_thread_id_t thread_id)
{
    if (omp_get_thread_num() == 0) {
        ASSERT(thread_type == ompt_thread_initial, IMPLEMENTED_BUT_INCORRECT, "Expect to see ompt_thread_initial");
    } else {
        ASSERT(thread_type == ompt_thread_worker, IMPLEMENTED_BUT_INCORRECT, "Expect to see ompt_thread_worker");
        ASSERT(thread_id_map.count(thread_id) == 0, IMPLEMENTED_BUT_INCORRECT, "Expect non-duplicate thread ids");
        thread_id_map[thread_id] = thread_type;
    }
}

void 
init_test(ompt_function_lookup_t lookup)
{
    if (!register_callback(ompt_event_thread_begin, (ompt_callback_t) on_ompt_event_thread_begin)) {
        cout << "Failed to register ompt_event_thread_begin" << endl;
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
    ASSERT(thread_id_map.size() == (NUM_THREADS), IMPLEMENTED_BUT_INCORRECT, "Wrong number of calls to ompt_event_threadbegins");
    return CORRECT;
}
