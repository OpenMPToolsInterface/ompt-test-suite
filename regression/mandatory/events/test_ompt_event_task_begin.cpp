#include <omp.h>
#include <common.h>
#include <iostream>
#include <sstream>      
#include <map>

using namespace std;
typedef map<ompt_task_id_t, ompt_task_id_t> threadIdMap_t;
static threadIdMap_t task_id_map;

ompt_task_id_t curr_task_id;
ompt_frame_t * curr_task_frame;

void on_ompt_event_task_begin(ompt_task_id_t parent_task_id,    
                              ompt_frame_t *parent_task_frame,  
                              ompt_task_id_t new_task_id,       
                              void *new_task_function)
{
    printf("1111111 %d %d \n", curr_task_id, parent_task_id);
    //ASSERT((parent_task_id == curr_task_id || parent_task_id == 0), IMPLEMENTED_BUT_INCORRECT, "Incorrect parent task id");
    //ASSERT(parent_task_frame == curr_task_frame, IMPLEMENTED_BUT_INCORRECT, "Incorrect parent task frame");
    //ASSERT(task_id_map.count(new_task_id) == 0, IMPLEMENTED_BUT_INCORRECT, "Duplicate task ids in a nested sequence");
    curr_task_id = new_task_id;
    task_id_map[new_task_id] = parent_task_id;
}

void 
init_test(ompt_function_lookup_t lookup)
{
    if (!register_callback(ompt_event_task_begin, (ompt_callback_t) on_ompt_event_task_begin)) {
        cout << "Failed to register ompt_event_task_begin" << endl;
        exit(NOT_IMPLEMENTED);
    }
}

int
main(int argc, char** argv)
{

    #pragma omp parallel num_threads(NUM_THREADS)
    {
        #pragma omp master
        {
            curr_task_id = ompt_get_task_id(0);
            curr_task_frame = ompt_get_task_frame(0);
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
    return CORRECT;
}
