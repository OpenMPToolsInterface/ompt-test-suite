#include <omp.h>
#include <common.h>
#include <iostream>
#include <sstream>      
#include <map>
#include <set>

#define NUM_THREADS 4

using namespace std;

int count = 0;
map<ompt_task_id_t, ompt_task_id_t> task_id_to_task_id_map;
map<ompt_task_id_t, ompt_frame_t *> task_id_to_task_frame_map;
set<ompt_task_id_t> task_ids;

void 
on_ompt_event_task_begin(ompt_task_id_t parent_task_id,    
                              ompt_frame_t *parent_task_frame,  
                              ompt_task_id_t new_task_id,       
                              void *new_task_function)
{
    task_id_to_task_id_map[new_task_id] = parent_task_id;
    task_id_to_task_frame_map[new_task_id] = parent_task_frame;
    task_ids.insert(new_task_id);
    #pragma omp atomic update
    count += 1;
}

void
on_ompt_event_task_end(ompt_task_id_t  task_id)
{
    CHECK(task_ids.count(task_id) != 0, IMPLEMENTED_BUT_INCORRECT, "no record for task id");
    #pragma omp atomic update
    count -= 1;
}

void 
init_test(ompt_function_lookup_t lookup)
{
    if (!register_callback(ompt_event_task_begin, (ompt_callback_t) on_ompt_event_task_begin)) {
        CHECK(false, FATAL, "failed to register ompt_event_task_begin");
    }
    if (!register_callback(ompt_event_task_end, (ompt_callback_t) on_ompt_event_task_end)) {
        CHECK(false, FATAL, "failed to register ompt_event_task_begin");
    }
}

int
main(int argc, char** argv)
{
    register_segv_handler(argv);
    warmup();
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
    CHECK(count == 0, IMPLEMENTED_BUT_INCORRECT, "unbalanced number of calls to begin and end callbacks");
    return global_error_code;
}
