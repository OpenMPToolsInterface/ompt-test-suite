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

void
on_ompt_event_parallel_begin(ompt_task_id_t parent_task_id,    /* id of parent task            */
                             ompt_frame_t *parent_task_frame,  /* frame data of parent task    */
                             ompt_parallel_id_t parallel_id,   /* id of parallel region        */
                             uint32_t requested_team_size,     /* number of threads in team    */
                             void *parallel_function           /* pointer to outlined function */)
{
    if (parallel_id_map.count(parallel_id) != 0) {
        error_flag = true;
        error_msg = "duplicated parallel region ids";
    }
    
    parallel_id_map[parallel_id] = parent_task_id;
    if (requested_team_size != NUM_THREADS) {
        error_flag = true;
        ostringstream stringStream;
        stringStream << "requested team size should be " << NUM_THREADS;
        error_msg  = stringStream.str();
    }
}

void 
init_test(ompt_function_lookup_t lookup)
{
    if (!register_callback(ompt_event_parallel_begin, (ompt_callback_t) on_ompt_event_parallel_begin)) {
        cout << "Failed to register ompt_event_parallel_begin" << endl;
        exit(NOT_IMPLEMENTED);
    }
    error_flag = false;
}

int
main(int argc, char** argv)
{
    omp_set_nested(3);
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
    
    if (parallel_id_map.size() != (1+(1+NUM_THREADS)*NUM_THREADS)) {
        error_msg = "Enter parallel incorrect number of times";
        error_flag = true;
    }

    if (error_flag) {
        cout << error_msg << endl;
        return IMPLEMENTED_BUT_INCORRECT;;
    }
    
    return CORRECT;
}
