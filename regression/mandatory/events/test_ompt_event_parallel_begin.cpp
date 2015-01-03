#include <omp.h>
#include <common.h>
#include <iostream>
#include <sstream>      
#include <map>

#include <pthread.h>

#define NUM_THREADS 2

using namespace std;

map<ompt_parallel_id_t, ompt_task_id_t> parallel_id_to_task_id_map;
map<ompt_parallel_id_t, ompt_frame_t *> parallel_id_to_task_frame_map;

ompt_task_id_t global_parent_task_id;
ompt_frame_t * global_parent_task_frame;
bool test_enclosing_context;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

volatile int regions_encountered = 0;

void
on_ompt_event_parallel_begin(ompt_task_id_t parent_task_id,    /* id of parent task            */
                             ompt_frame_t *parent_task_frame,  /* frame data of parent task    */
                             ompt_parallel_id_t parallel_id,   /* id of parallel region        */
                             uint32_t requested_team_size,     /* number of threads in team    */
                             void *parallel_function           /* pointer to outlined function */)
{
    CHECK(parallel_id_to_task_id_map.count(parallel_id) == 0, IMPLEMENTED_BUT_INCORRECT, "duplicated parallel region ids");
    CHECK(requested_team_size == NUM_THREADS, IMPLEMENTED_BUT_INCORRECT, "wrong requested team size");

    pthread_mutex_lock(&mutex);
    parallel_id_to_task_id_map[parallel_id] = parent_task_id;
    parallel_id_to_task_frame_map[parallel_id] = parent_task_frame;
    pthread_mutex_unlock(&mutex);

    if (test_enclosing_context) {
        CHECK(ompt_get_task_id(0) == global_parent_task_id, IMPLEMENTED_BUT_INCORRECT,\
              "parallel begin callback doesn't execute in parent's context");
        CHECK(ompt_get_task_frame(0) == global_parent_task_frame, IMPLEMENTED_BUT_INCORRECT,\
              "parallel begin callback doesn't execute in parent's context");
    }
}

void 
init_test(ompt_function_lookup_t lookup)
{
    if (!register_callback(ompt_event_parallel_begin, (ompt_callback_t) on_ompt_event_parallel_begin)) {
        CHECK(false, FATAL, "failed to register ompt_event_parallel_begin");
    }
}

int
main(int argc, char** argv)
{
    register_segv_handler(argv);
    warmup();
    pthread_mutex_unlock(&mutex);

    /* test whether callback executes in parent enclosing context */
    test_enclosing_context = true;
    global_parent_task_id = ompt_get_task_id(0);
    global_parent_task_frame = ompt_get_task_frame(0);
    #pragma omp parallel num_threads(NUM_THREADS)
    {
        serialwork(0);
    }
    test_enclosing_context = false; 
    parallel_id_to_task_id_map.clear();
    parallel_id_to_task_frame_map.clear();
    
    omp_set_nested(3);
    regions_encountered += 1;
    #pragma omp parallel num_threads(NUM_THREADS)
    {
        serialwork(0);
        ompt_parallel_id_t level1_parallel_id = ompt_get_parallel_id(0);
        ompt_task_id_t   level1_task_id = ompt_get_task_id(0);
        CHECK(ompt_get_task_id(1) == parallel_id_to_task_id_map[level1_parallel_id], IMPLEMENTED_BUT_INCORRECT, 
	      "level 1 parent task id does not match");
        CHECK(ompt_get_task_frame(1) == parallel_id_to_task_frame_map[level1_parallel_id], IMPLEMENTED_BUT_INCORRECT, 
	      "level 1 parent task frame does not match");

        #pragma omp atomic update
        regions_encountered += 1;
        #pragma omp parallel num_threads(NUM_THREADS)
        {
            serialwork(0);
            ompt_parallel_id_t level2_parallel_id = ompt_get_parallel_id(0);
            CHECK(ompt_get_task_id(1) == parallel_id_to_task_id_map[level2_parallel_id], IMPLEMENTED_BUT_INCORRECT, 
	          "level 2 parent task id does not match");
            CHECK(ompt_get_task_frame(1) == parallel_id_to_task_frame_map[level2_parallel_id], 
                  IMPLEMENTED_BUT_INCORRECT, "level 2 parent task frame does not match");

            #pragma omp atomic update
            regions_encountered += 1;
            #pragma omp parallel num_threads(NUM_THREADS)
            {
                serialwork(0);
            }
        }
    }
    
    int begins = parallel_id_to_task_id_map.size();
    CHECK(begins == regions_encountered, IMPLEMENTED_BUT_INCORRECT, \
          "parallel region begin doesn't match region entries (expected %d observed %d)", \
          regions_encountered, begins);
    return global_error_code;
}
