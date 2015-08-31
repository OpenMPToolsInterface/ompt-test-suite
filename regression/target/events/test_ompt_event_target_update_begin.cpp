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

static void on_ompt_event_target_update_begin(ompt_task_id_t parent_task_id,
                ompt_frame_t *parent_task_frame,
                ompt_task_id_t target_task_id,
                int device_id,
                void *target_task_function) {
#if DEBUG
    printf("begin: parent_task_id = %" PRIu64 ", parent_task_frame = %p, target_task_id = %" PRIu64 ", device_id = %" PRIu64 ", target_task_function = %p\n",
        parent_task_id, parent_task_frame, target_task_id, device_id, target_task_function);
#endif

    CHECK(parent_task_id > 0, IMPLEMENTED_BUT_INCORRECT, "invalid parent_task_id");
    CHECK(target_task_id > 0, IMPLEMENTED_BUT_INCORRECT, "invalid target_task_id");
    CHECK(target_task_id == ompt_get_task_id(0), IMPLEMENTED_BUT_INCORRECT, "target_task_id not equal to ompt_get_task_id()");

    pthread_mutex_lock(&thread_mutex);
    number_begin_events += 1;
    pthread_mutex_unlock(&thread_mutex);
}


void init_test(ompt_function_lookup_t lookup)
{

#if defined(_OPENMP) && (_OPENMP >= 201307)
    if (!register_callback(ompt_event_target_update_begin, (ompt_callback_t) on_ompt_event_target_update_begin)) {
        CHECK(false, FATAL, "failed to register ompt_event_target_update_begin");
    }
#endif

}

//*****************************************************************************
// interface operations
//*****************************************************************************

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

        CHECK(a == 2, IMPLEMENTED_BUT_INCORRECT, "Update from device to host failed");
    }

    CHECK(number_begin_events == 1, IMPLEMENTED_BUT_INCORRECT,  "number of update_begin events not as expected (expected: %d, oberved: %d)", 1, number_begin_events);

    return return_code;
#else
    CHECK(FALSE, NOT_IMPLEMENTED, "OpenMP 4.0 not supported; OpenMP TARGET feature not tested");
    return NOT_IMPLEMENTED;
#endif

}
