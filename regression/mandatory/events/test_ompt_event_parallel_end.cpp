#include <omp.h>
#include <common.h>
#include <iostream>
#include <sstream>      
#include <map>

using namespace std;

typedef map<ompt_task_id_t, ompt_task_id_t> parallel_id_map_t;
parallel_id_map_t parallel_id_map;
bool error_flag;
string error_msg;
int count = 0;

void
on_ompt_event_parallel_begin(ompt_task_id_t parent_task_id,    /* id of parent task            */
                             ompt_frame_t *parent_task_frame,  /* frame data of parent task    */
                             ompt_parallel_id_t parallel_id,   /* id of parallel region        */
                             uint32_t requested_team_size,     /* number of pregions in team    */
                             void *parallel_function           /* pointer to outlined function */)
{
    parallel_id_map[parallel_id] = parent_task_id;
    #pragma omp atomic update
    count += 1;
}

void
on_ompt_event_parallel_end(ompt_parallel_id_t parallel_id,    /* id of parallel region       */
                ompt_task_id_t task_id             /* id of task                  */)
{
    if (parallel_id_map.count(parallel_id) == 0) {
        error_flag = true;
        ostringstream stringStream;
        stringStream << "No record found for parallel id " << parallel_id;
        error_msg  = stringStream.str();
    }
    #pragma omp atomic update
    count -= 1;
}

void 
init_test(ompt_function_lookup_t lookup)
{
    if (!register_callback(ompt_event_parallel_begin, (ompt_callback_t) on_ompt_event_parallel_begin)) {
        cout << "Failed to register ompt_event_parallel_begin" << endl;
        exit(NOT_IMPLEMENTED);
    }
    if (!register_callback(ompt_event_parallel_end, (ompt_callback_t) on_ompt_event_parallel_end)) {
        cout << "Failed to register ompt_event_parallel_end" << endl;
        exit(NOT_IMPLEMENTED);
    }
    error_flag = false;
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

    if (count != 0) {
        error_flag = true;
        error_msg = "Number of calls to parallel begin differs from the number of calls to end";
    }

    if (error_flag) {
        cout << error_msg << endl;
        return IMPLEMENTED_BUT_INCORRECT;;
    }
    
    return CORRECT;
}
