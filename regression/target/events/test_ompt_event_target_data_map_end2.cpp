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

int count = 0; // target_data_map_begin -> increased, target_data_map_end -> decreased
int number_begin_calls = 0;

// save task_id for a target_data_map_begin 
ompt_task_id_t begin_task_id;


//*****************************************************************************
// private operations
//*****************************************************************************

static void on_ompt_event_target_data_map_begin(ompt_task_id_t task_id,
                int device_id,
                void *host_addr,
                void *device_addr,
                size_t bytes,
                uint32_t mapping_flags,
                void *target_map_code) {
#if DEBUG
    printf("task_id = %" PRIu64 ", device_id = %" PRIu64 ", host_addr = %p, device_addr = %p, bytes = %" PRIu64 ", mapping_flags = %" PRIu32 ", target_map_code = %p\n",
        task_id, device_id, host_addr, device_addr, bytes, mapping_flags, target_map_code);
#endif

    CHECK(task_id > 0, IMPLEMENTED_BUT_INCORRECT, "invalid task_id");
    CHECK(task_id == ompt_get_task_id(0), IMPLEMENTED_BUT_INCORRECT, "task_id not equal to ompt_get_task_id()");

    pthread_mutex_lock(&thread_mutex);

    // save task_id for current thread
    begin_task_id = task_id;

    count += 1;
    number_begin_calls += 1;

    pthread_mutex_unlock(&thread_mutex);
}

static void on_ompt_event_target_data_map_end(ompt_task_id_t task_id) {
    pthread_mutex_lock(&thread_mutex);

#if DEBUG
    printf("end: task_id = %" PRIu64 "\n", task_id);
#endif

    CHECK(task_id > 0, IMPLEMENTED_BUT_INCORRECT, "invalid task_id");
    CHECK(task_id == ompt_get_task_id(0), IMPLEMENTED_BUT_INCORRECT, "task_id not equal to ompt_get_task_id()");

    // check for correct task_id in target_data_map_end
    // (should be the same as in target_data_map_begin for current thread)
    CHECK(begin_task_id == task_id, IMPLEMENTED_BUT_INCORRECT, "task_ids not equal"); 

    count -= 1;

    pthread_mutex_unlock(&thread_mutex);
}


//*****************************************************************************
// interface operations
//*****************************************************************************

void init_test(ompt_function_lookup_t lookup) {

#if defined(_OPENMP) && (_OPENMP >= 201307)
    if (!register_callback(ompt_event_target_data_map_begin, (ompt_callback_t) on_ompt_event_target_data_map_begin)) {
        CHECK(false, FATAL, "failed to register ompt_event_target_data_map_begin");
    }

    if (!register_callback(ompt_event_target_data_map_end, (ompt_callback_t) on_ompt_event_target_data_map_end)) {
        CHECK(false, FATAL, "failed to register ompt_event_target_data_map_end");
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
