//*****************************************************************************
// OpenMP runtime includes 
//*****************************************************************************

#include <omp.h>
#include <stdio.h>


//*****************************************************************************
// regression harness includes
//*****************************************************************************

#include <ompt-regression.h>
#include <ompt-initialize.h>


//*****************************************************************************
// global variables
//*****************************************************************************

int count = 0; // target_update_begin -> increased, target_update_end -> decreased
int number_begin_events = 0;

// save task_id for a target_update_begin 
ompt_task_id_t begin_task_id;


//*****************************************************************************
// function to be implemented by each test
//*****************************************************************************

void update_end_test();


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
    count += 1;

    // save target_task_id for current thread
    begin_task_id = target_task_id;

    pthread_mutex_unlock(&thread_mutex);
}


static void on_ompt_event_target_update_end(ompt_task_id_t task_id) {
    pthread_mutex_lock(&thread_mutex);

#if DEBUG
    printf("end: task_id = %" PRIu64 "\n", task_id);
#endif

    CHECK(task_id > 0, IMPLEMENTED_BUT_INCORRECT, "invalid task_id");
    CHECK(task_id == ompt_get_task_id(0), IMPLEMENTED_BUT_INCORRECT, "task_id not equal to ompt_get_task_id()");

    // Check for correct task_id in target_update_end
    // (should be the same as in target_update_begin)
    CHECK(begin_task_id == task_id, IMPLEMENTED_BUT_INCORRECT, "task_ids not equal"); 

    count -= 1;

    pthread_mutex_unlock(&thread_mutex);
}


//*****************************************************************************
// interface operations
//*****************************************************************************

void init_test(ompt_function_lookup_t lookup) {

#if defined(_OPENMP) && (_OPENMP >= 201307)
    if (!register_callback(ompt_event_target_update_begin, (ompt_callback_t) on_ompt_event_target_update_begin)) {
        CHECK(false, FATAL, "failed to register ompt_event_target_update_begin");
    }

    if (!register_callback(ompt_event_target_update_end, (ompt_callback_t) on_ompt_event_target_update_end)) {
        CHECK(false, FATAL, "failed to register ompt_event_target_update_end");
    }
#endif

}

int regression_test(int argc, char **argv) {

#if defined(_OPENMP) && (_OPENMP >= 201307)
    // call specific test function
    update_end_test();

    CHECK(count == 0, IMPLEMENTED_BUT_INCORRECT,  "not the same number of target_update_begin and target_update_end calls (count = %d)", count);

    return return_code;
#else
    CHECK(FALSE, NOT_IMPLEMENTED, "OpenMP 4.0 not supported; OpenMP TARGET feature not tested");
    return NOT_IMPLEMENTED;
#endif

}
