#include <omp.h>
#include <common.h>
#include <iostream>
#include <sstream>      
#include <map>

#define NUM_THREADS 4

using namespace std;

map<ompt_task_id_t, ompt_task_id_t> task_id_to_task_id_map;
map<ompt_task_id_t, ompt_frame_t *> task_id_to_task_frame_map;


void on_ompt_event_task_begin(ompt_task_id_t parent_task_id,    
                              ompt_frame_t *parent_task_frame,  
                              ompt_task_id_t new_task_id,       
                              void *new_task_function)
{
    CHECK(ompt_get_task_id(0) == parent_task_id, IMPLEMENTED_BUT_INCORRECT, \
                                      "Task begin callback doesn't execute in parent's context")
    CHECK(ompt_get_task_frame(0) == parent_task_frame, IMPLEMENTED_BUT_INCORRECT, \
                                     "Task begin callback doesn't execute in parent's context")
    CHECK(task_id_to_task_id_map.count(new_task_id) == 0, IMPLEMENTED_BUT_INCORRECT, "duplicated task ids");
    task_id_to_task_id_map[new_task_id] = parent_task_id;
    task_id_to_task_frame_map[new_task_id] = parent_task_frame;
}

void 
init_test(ompt_function_lookup_t lookup)
{
    if (!register_callback(ompt_event_task_begin, (ompt_callback_t) on_ompt_event_task_begin)) {
        CHECK(false, NOT_IMPLEMENTED, "Failed to register ompt_event_task_begin");
    }
}

int
main(int argc, char** argv)
{
    warmup();
    #pragma omp parallel num_threads(NUM_THREADS)
    {
        #pragma omp master
        {
            #pragma omp task
            {
                serialwork(0);
                ompt_task_id_t  level1_task_id = ompt_get_task_id(0);
                CHECK(ompt_get_task_id(1) == task_id_to_task_id_map[level1_task_id], \
                      IMPLEMENTED_BUT_INCORRECT, \
                      "Level 1 parent task id does not match");
                CHECK(ompt_get_task_frame(1) == task_id_to_task_frame_map[level1_task_id], \
                      IMPLEMENTED_BUT_INCORRECT, \
                      "Level 1 parent task frame does not match");
                #pragma omp task
                {
                    serialwork(0);
                    ompt_task_id_t  level2_task_id = ompt_get_task_id(0);
                    CHECK(ompt_get_task_id(1) == task_id_to_task_id_map[level2_task_id], \
                          IMPLEMENTED_BUT_INCORRECT, \
                          "Level 2 parent task id does not match");
                    CHECK(ompt_get_task_frame(1) == task_id_to_task_frame_map[level2_task_id], \
                          IMPLEMENTED_BUT_INCORRECT, \
                          "Level 2 parent task frame does not match");
                    #pragma omp task
                    {
                        serialwork(1);
                    }
                }
            }
        }
    }
    return global_error_code;
}
