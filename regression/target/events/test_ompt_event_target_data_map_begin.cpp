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

int number_begin_events = 0;


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
    number_begin_events += 1;
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
#endif

}

int regression_test(int argc, char **argv) {

#if defined(_OPENMP) && (_OPENMP >= 201307)
    int a = 1;
    #pragma omp target data map(to: a)
    {
        #pragma omp target
        {
            a = 2;
        }
        
        #pragma omp target update from(a)
    }

    CHECK(number_begin_events == 2, IMPLEMENTED_BUT_INCORRECT,  "number of data_map_begin events not as expected (expected: %d, oberved: %d)", 2, number_begin_events);

    return return_code;
#else
    CHECK(FALSE, NOT_IMPLEMENTED, "OpenMP 4.0 not supported; OpenMP TARGET feature not tested");
    return NOT_IMPLEMENTED;
#endif

}
