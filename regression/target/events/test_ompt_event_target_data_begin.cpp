//*****************************************************************************
// system includes 
//*****************************************************************************

#include <stdlib.h>


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

static void on_ompt_event_target_data_begin(ompt_task_id_t task_id,
                ompt_target_id_t target_id,
                ompt_target_device_id_t device_id,
                void* target_function) {
#if DEBUG
    printf("task_id = %" PRIu64 ", target_id = %" PRIu64 ", device_id = %" PRIu64 ", target_function = %p\n",
        task_id, target_id, device_id, target_function);
#endif

    CHECK(task_id > 0, IMPLEMENTED_BUT_INCORRECT, "invalid task_id");
    CHECK(task_id == ompt_get_task_id(0), IMPLEMENTED_BUT_INCORRECT, "task_id not equal to ompt_get_task_id()");    

    pthread_mutex_lock(&thread_mutex);
    number_begin_events += 1;
    pthread_mutex_unlock(&thread_mutex);
} 

void init_test(ompt_function_lookup_t lookup) {
#if defined(_OPENMP) && (_OPENMP >= 201307)
    if (!register_callback(ompt_event_target_data_begin, (ompt_callback_t) on_ompt_event_target_data_begin)) {
        CHECK(false, FATAL, "failed to register ompt_event_target_update_begin");
    }
#endif

}


//*****************************************************************************
// interface operations
//*****************************************************************************

int regression_test(int argc, char **argv) {

#if defined(_OPENMP) && (_OPENMP >= 201307)
    // empty data region
    #pragma omp target data
    {
        sleep(1);
    }

    CHECK(number_begin_events == 1, IMPLEMENTED_BUT_INCORRECT,  "number of target_data_begin events not as expected (expected: %d, oberved: %d)", 1, number_begin_events);

    return return_code;
#else
    CHECK(FALSE, NOT_IMPLEMENTED, "OpenMP 4.0 not supported; OpenMP TARGET feature not tested");
    return NOT_IMPLEMENTED;
#endif

}