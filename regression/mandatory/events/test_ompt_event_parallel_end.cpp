#include <omp.h>
#include <common.h>
#include <iostream>
#include <sstream>      
#include <map>
#include <set>

#include <pthread.h>

#define DEBUG 0

#define NUM_THREADS 4

using namespace std;
static map<ompt_parallel_id_t, ompt_task_id_t> parallel_id_to_task_id;
static set<ompt_task_id_t> task_ids;

int count = 0;
ompt_task_id_t serial_task_id;
ompt_frame_t * serial_task_frame;
bool test_enclosing_context;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


void
on_ompt_event_parallel_begin(ompt_task_id_t parent_task_id,    /* id of parent task            */
                             ompt_frame_t *parent_task_frame,  /* frame data of parent task    */
                             ompt_parallel_id_t parallel_id,   /* id of parallel region        */
                             uint32_t requested_team_size,     /* number of pregions in team    */
                             void *parallel_function           /* pointer to outlined function */)
{
    pthread_mutex_lock(&mutex);
#if DEBUG
    printf("parallel_id = %lld, parent_task_frame %p, parent_task_id = %lld\n", 
           parallel_id, parent_task_frame, parent_task_id);
#endif
    parallel_id_to_task_id[parallel_id] = parent_task_id;
    task_ids.insert(parent_task_id);
    count += 1;
    pthread_mutex_unlock(&mutex);
}

void
on_ompt_event_parallel_end(ompt_parallel_id_t parallel_id,    /* id of parallel region       */
                ompt_task_id_t task_id                        /* id of task                  */)
{
    pthread_mutex_lock(&mutex);
#if DEBUG
    printf("parallel_id = %lld, task_id = %lld\n", parallel_id, task_id);
#endif
    CHECK(parallel_id_to_task_id.count(parallel_id) != 0, IMPLEMENTED_BUT_INCORRECT, \
          "no record found for parallel id");

    CHECK(task_ids.count(task_id) != 0, IMPLEMENTED_BUT_INCORRECT, \
          "end for task_id %lld with no matching begin", task_id);

    parallel_id_to_task_id.erase(parallel_id);
    task_ids.erase(task_id);
    count -= 1;
    pthread_mutex_unlock(&mutex);

    if (test_enclosing_context) {
        CHECK(ompt_get_task_id(0) == serial_task_id, IMPLEMENTED_BUT_INCORRECT,\
              "parallel end callback doesn't execute in parent's context");
        CHECK(ompt_get_task_frame(0) == serial_task_frame, IMPLEMENTED_BUT_INCORRECT,\
              "parallel end callback doesn't execute in parent's context");
    }
}

void 
init_test(ompt_function_lookup_t lookup)
{
    if (!register_callback(ompt_event_parallel_begin, (ompt_callback_t) on_ompt_event_parallel_begin)) {
        CHECK(FALSE, FATAL, "failed to register ompt_event_parallel_begin");
    }
    if (!register_callback(ompt_event_parallel_end, (ompt_callback_t) on_ompt_event_parallel_end)) {
        CHECK(FALSE, FATAL, "failed to register ompt_event_parallel_end");
    }
}

int
main(int argc, char** argv)
{
    register_segv_handler(argv);
    warmup();

    serial_task_id = ompt_get_task_id(0);
    serial_task_frame = ompt_get_task_frame(0);

#if DEBUG
    printf("serial_task_id = %lld, serial_task_frame = %p\n", 
           serial_task_id, serial_task_frame);
#endif

    test_enclosing_context = true;
    #pragma omp parallel num_threads(NUM_THREADS)
    {
        serialwork(0);
    }
    test_enclosing_context = false;
    parallel_id_to_task_id.clear();

    CHECK(count == 0, IMPLEMENTED_BUT_INCORRECT, 
          "number of calls to parallel begin differs from the number of calls to end");
    return global_error_code;
}
