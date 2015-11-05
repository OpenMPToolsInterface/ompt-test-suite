//*****************************************************************************
// system includes 
//*****************************************************************************

#include <vector>

//*****************************************************************************
// OpenMP runtime includes 
//*****************************************************************************

#include <omp.h>


//*****************************************************************************
// regression harness includes
//*****************************************************************************

#include <ompt-regression.h>
#include <ompt-initialize.h>


//*****************************************************************************
// macros
//*****************************************************************************

#define DEBUG 0

#define NUM_THREADS 4


//*****************************************************************************
// global variables
//*****************************************************************************

int count = 0; // target_task_begin -> increased, target_task_end -> decreased
int number_begin_events = 0;

// save target_task_id for a target_task_begin (for each thread)
std::vector<ompt_task_id_t> target_task_ids(NUM_THREADS, 0);


//*****************************************************************************
// private operations
//*****************************************************************************
    
static void on_ompt_event_target_task_begin(ompt_task_id_t parent_task_id,
                ompt_frame_t *parent_task_frame,
                ompt_task_id_t host_task_id,
                int device_id,
                void *target_task_code,
                ompt_target_task_type_t task_type) {
    pthread_mutex_lock(&thread_mutex);

#if DEBUG
    printf("parent_task_id = %" PRIu64 ", parent_task_frame = %p, host_task_id = %" PRIu64 ", device_id = %" PRIu64 ", target_task_function = %p\n",
        parent_task_id, parent_task_frame, host_task_id, device_id, target_task_function);
#endif

    CHECK(parent_task_id > 0, IMPLEMENTED_BUT_INCORRECT, "invalid parent_task_id");
    CHECK(host_task_id > 0, IMPLEMENTED_BUT_INCORRECT, "invalid host_task_id");
    CHECK(host_task_id == ompt_get_task_id(0), IMPLEMENTED_BUT_INCORRECT, "host_task_id not equal to ompt_get_task_id()");
    CHECK(task_type == ompt_target_task_target, IMPLEMENTED_BUT_INCORRECT, "task_type not ompt_target_task_update");

    // save task_id for current thread
    target_task_ids[ompt_get_thread_id()] = host_task_id;

    count += 1;
    number_begin_events += 1;

    pthread_mutex_unlock(&thread_mutex);
}

static void on_ompt_event_target_task_end(ompt_task_id_t host_task_id) {
    pthread_mutex_lock(&thread_mutex);

#if DEBUG
    printf("end: target_task_id = %" PRIu64 "\n", target_task_id);
#endif

    CHECK(host_task_id > 0, IMPLEMENTED_BUT_INCORRECT, "invalid host_task_id");
    CHECK(host_task_id == ompt_get_task_id(0), IMPLEMENTED_BUT_INCORRECT, "host_task_id not equal to ompt_get_task_id()");


    // check for correct task_id in target_task_end
    // (should be the same as in target_task_begin for current thread)
    CHECK(target_task_ids[ompt_get_thread_id()] == host_task_id, IMPLEMENTED_BUT_INCORRECT, "task_ids not equal");

    count -= 1;

    pthread_mutex_unlock(&thread_mutex);
}

void init_test(ompt_function_lookup_t lookup) {

#if defined(_OPENMP) && (_OPENMP >= 201307)
    if (!register_callback(ompt_event_target_task_begin, (ompt_callback_t) on_ompt_event_target_task_begin)) {
        CHECK(false, FATAL, "failed to register ompt_event_target_task_begin");
    }

    if (!register_callback(ompt_event_target_task_end, (ompt_callback_t) on_ompt_event_target_task_end)) {
        CHECK(false, FATAL, "failed to register ompt_event_target_task_end");
    }
#endif

}

//*****************************************************************************
// interface operations
//*****************************************************************************

int regression_test(int argc, char **argv) {

#if defined(_OPENMP) && (_OPENMP >= 201307)
    #pragma omp parallel num_threads(NUM_THREADS)
    {
        #pragma omp target  
        {
            sleep(1);
        }

        #pragma omp target    
        {
            sleep(1);
        }
    }

    CHECK(count == 0, IMPLEMENTED_BUT_INCORRECT,  "not the same number of target_task_begin and target_task_end calls");
    CHECK(number_begin_events == 2*NUM_THREADS, IMPLEMENTED_BUT_INCORRECT,  "number of target_task_begin events not as expected (expected: %d, oberved: %d)", 2*NUM_THREADS, number_begin_events);

    return return_code;
#else
    CHECK(FALSE, NOT_IMPLEMENTED, "OpenMP 4.0 not supported; OpenMP TARGET feature not tested");
    return NOT_IMPLEMENTED;
#endif
}

