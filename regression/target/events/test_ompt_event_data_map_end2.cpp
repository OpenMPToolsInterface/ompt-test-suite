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


//*****************************************************************************
// global variables
//*****************************************************************************

int count = 0; // target_begin -> increased, target_end -> decreased
int number_begin_calls = 0;

// save target_id and corresponding task_id for a target_begin 
ompt_target_id_t begin_target_id;
ompt_task_id_t begin_task_id;


//*****************************************************************************
// private operations
//*****************************************************************************

static void on_ompt_event_data_map_begin(ompt_task_id_t task_id,
                ompt_target_id_t target_id,
                ompt_data_map_id_t data_map_id,
                ompt_target_device_id_t device_id,
                ompt_target_sync_t sync_type,
                ompt_data_map_t map_type,
                ompt_data_size_t bytes) {
    pthread_mutex_lock(&thread_mutex);

#if DEBUG
    printf("task_id = %lld, target_id = %lld, data_map_id = %lld, device_id = %lld, sync_type = %lld, map_type = %lld, bytes = %lld\n",
        task_id, target_id, data_map_id, device_id, sync_type, map_type, bytes);
#endif

    CHECK(task_id > 0, IMPLEMENTED_BUT_INCORRECT, "invalid task_id");
    CHECK(task_id == ompt_get_task_id(0), IMPLEMENTED_BUT_INCORRECT, "task_id not equal to ompt_get_task_id()");

    // save task_id and target_id for current thread
    begin_target_id = target_id;
    begin_task_id = task_id;

    count += 1;
    number_begin_calls += 1;

    pthread_mutex_unlock(&thread_mutex);
}

static void on_ompt_event_data_map_end(ompt_task_id_t task_id,
                  ompt_target_id_t target_id) {
    pthread_mutex_lock(&thread_mutex);

#if DEBUG
    printf("end: task_id = %lld, target_id = %lld\n", task_id, target_id);
#endif

    CHECK(task_id > 0, IMPLEMENTED_BUT_INCORRECT, "invalid task_id");
    CHECK(task_id == ompt_get_task_id(0), IMPLEMENTED_BUT_INCORRECT, "task_id not equal to ompt_get_task_id()");

    // check for correct target_id and task_id in target_end
    // (should be the same as in target_begin for current thread)
    CHECK(begin_target_id == target_id, IMPLEMENTED_BUT_INCORRECT, "target_ids not equal");
    CHECK(begin_task_id == task_id, IMPLEMENTED_BUT_INCORRECT, "task_ids not equal"); 

    count -= 1;

    pthread_mutex_unlock(&thread_mutex);
}


//*****************************************************************************
// interface operations
//*****************************************************************************

void init_test(ompt_function_lookup_t lookup) {

#if defined(_OPENMP) && (_OPENMP >= 201307)
    if (!register_callback(ompt_event_data_map_begin, (ompt_callback_t) on_ompt_event_data_map_begin)) {
        CHECK(false, FATAL, "failed to register ompt_event_data_map_begin");
    }

    if (!register_callback(ompt_event_data_map_end, (ompt_callback_t) on_ompt_event_data_map_end)) {
        CHECK(false, FATAL, "failed to register ompt_event_data_map_end");
    }
#endif

}

int regression_test(int argc, char **argv) {

#if defined(_OPENMP) && (_OPENMP >= 201307)
    int a = 1;

    #pragma omp target data map(alloc: a)
    {
        #pragma omp target
        {
            a = 1;
        }

        #pragma omp target update to(a)
    } 

    CHECK(number_begin_calls == 1, IMPLEMENTED_BUT_INCORRECT,  "number of data_map_begin events not as expected (expected: %d, oberved: %d)", 1, number_begin_calls);

    CHECK(count == 0, IMPLEMENTED_BUT_INCORRECT,  "not the same number of data_map_begin and data_map_end calls");

    return return_code;
#else
    CHECK(FALSE, NOT_IMPLEMENTED, "OpenMP 4.0 not supported; OpenMP TARGET feature not tested");
    return NOT_IMPLEMENTED;
#endif

}
