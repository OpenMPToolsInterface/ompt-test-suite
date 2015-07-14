//*****************************************************************************
// system includes 
//*****************************************************************************

#include <stack>


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
// global variables
//*****************************************************************************

int count = 0; // target_begin -> increased, target_end -> decreased
int number_begin_events = 0;

// use stack to save task_ids and target_ids (especially for nested regions) 
std::stack<ompt_task_id_t> task_id_stack;
std::stack<ompt_target_id_t> target_id_stack;


//*****************************************************************************
// function to be implemented by each test
//*****************************************************************************

void data_end_test();


//*****************************************************************************
// private operations
//*****************************************************************************

static void on_ompt_event_target_data_begin(ompt_task_id_t task_id,
                ompt_target_id_t target_id,
                ompt_target_device_id_t device_id,
                void* target_function) {
    pthread_mutex_lock(&thread_mutex);

#if DEBUG
    printf("begin: task_id = %" PRIu64 ", target_id = %" PRIu64 ", device_id = %" PRIu64 ", target_function = %p\n",
        task_id, target_id, device_id, target_function);
#endif

    CHECK(task_id > 0, IMPLEMENTED_BUT_INCORRECT, "invalid task_id");
    CHECK(task_id == ompt_get_task_id(0), IMPLEMENTED_BUT_INCORRECT, "task_id not equal to ompt_get_task_id()");

    // save task_id and target_id
    task_id_stack.push(task_id);
    target_id_stack.push(target_id);

    count += 1;
    number_begin_events += 1;

    pthread_mutex_unlock(&thread_mutex);
}

static void on_ompt_event_target_data_end(ompt_task_id_t task_id,
                  ompt_target_id_t target_id) {
    pthread_mutex_lock(&thread_mutex);

#if DEBUG
    printf("end: task_id = %" PRIu64 ", target_id = %" PRIu64 "\n",
        task_id, target_id);
#endif

    CHECK(task_id > 0, IMPLEMENTED_BUT_INCORRECT, "invalid task_id");
    CHECK(task_id == ompt_get_task_id(0), IMPLEMENTED_BUT_INCORRECT, "task_id not equal to ompt_get_task_id()");

    // check for correct target_id and task_id in target_end
    // (should be the same as in target_begin)
    CHECK(task_id_stack.top() == task_id, IMPLEMENTED_BUT_INCORRECT, "task_ids not equal"); 
    CHECK(target_id_stack.top() == target_id, IMPLEMENTED_BUT_INCORRECT, "target_ids not equal"); 

    task_id_stack.pop();
    target_id_stack.pop();

    count -= 1;

    pthread_mutex_unlock(&thread_mutex);
}


//*****************************************************************************
// interface operations
//*****************************************************************************

void init_test(ompt_function_lookup_t lookup) {

#if defined(_OPENMP) && (_OPENMP >= 201307)
    if (!register_callback(ompt_event_target_data_begin, (ompt_callback_t) on_ompt_event_target_data_begin)) {
        CHECK(false, NOT_IMPLEMENTED, "failed to register ompt_event_target_data_begin");
    }

    if (!register_callback(ompt_event_target_data_end, (ompt_callback_t) on_ompt_event_target_data_end)) {
        CHECK(false, NOT_IMPLEMENTED, "failed to register ompt_event_target_data_end");
    }
#endif

}


int regression_test(int argc, char **argv) {

#if defined(_OPENMP) && (_OPENMP >= 201307)
    // call specific test function
    data_end_test();

    CHECK(count == 0, IMPLEMENTED_BUT_INCORRECT, "not the same number of target_data_begin and target_data_end calls (count = %d)", count);

    return return_code;
#else
    CHECK(FALSE, NOT_IMPLEMENTED, "OpenMP 4.0 not supported; OpenMP TARGET feature not tested");
    return NOT_IMPLEMENTED;
#endif

}
