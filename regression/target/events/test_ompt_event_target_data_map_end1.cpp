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
int number_items = 0;

// save task_id for a target_data_map_begin 
ompt_target_activity_id_t begin_map_id;


//*****************************************************************************
// private operations
//*****************************************************************************

static void on_ompt_event_target_data_map_begin(ompt_task_id_t task_id,
                int device_id,
                ompt_target_map_entry_t *items,
                uint32_t nitems,
                ompt_target_activity_id_t map_id) {
#if DEBUG
    printf("task_id = %" PRIu64 ", device_id = %d, nitems = %" PRIu32 ", map_id = %" PRIu64 "\n",
        task_id, device_id, nitems, map_id);
#endif

    CHECK(task_id > 0, IMPLEMENTED_BUT_INCORRECT, "invalid task_id");
    CHECK(task_id == ompt_get_task_id(0), IMPLEMENTED_BUT_INCORRECT, "task_id not equal to ompt_get_task_id()");

    pthread_mutex_lock(&thread_mutex);

    // save task_id for current thread
    begin_map_id = map_id;

    count += 1;
    number_begin_calls += 1;
    number_items += nitems;

    pthread_mutex_unlock(&thread_mutex);
}

static void on_ompt_event_target_data_map_end(int device_id,
                ompt_target_activity_id_t map_id) {
    pthread_mutex_lock(&thread_mutex);

#if DEBUG
    printf("end: device_id = %d, map_id = %" PRIu64 "\n", device_id, map_id);
#endif

    // check for correct map_id in target_data_map_end
    // (should be the same as in target_data_map_begin for current thread)
    CHECK(begin_map_id == map_id, IMPLEMENTED_BUT_INCORRECT, "map_ids not equal"); 

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
    int a;
    // start value on host
    a = 1;
    
    // save old values for error checking
    int a_old_device, a_old_host;

    #pragma omp target data map(alloc: a, a_old_device)
    {
        // save old a on host
        a_old_host = a;

        #pragma omp target
        {
            // modify a on device
            a = 42;

            // save modified a on device
            a_old_device = a;
        }

        // copy new value to host
        #pragma omp target update from(a)

        CHECK(a_old_host != a, IMPLEMENTED_BUT_INCORRECT, "update from device to host not working correctly");

        // modify a on host
        a = 0;
        // copy new value to device
        #pragma omp target update to(a)
    
        bool device_ok = true; // CHECK-directive does not work in target region, so use bool variable
        #pragma omp target
        {
            if (a_old_device == a) {
                device_ok = false;
            }
        }
        CHECK(device_ok, IMPLEMENTED_BUT_INCORRECT, "update from host to device not working correctly");
    }

    CHECK(number_begin_calls == 4, IMPLEMENTED_BUT_INCORRECT,  "number of data_map_begin events not as expected (expected: %d, oberved: %d)", 4, number_begin_calls);
    CHECK(number_items == 4, IMPLEMENTED_BUT_INCORRECT,  "total sum of nitems not as expected (expected: %d, oberved: %d)", 4, number_items);

    CHECK(count == 0, IMPLEMENTED_BUT_INCORRECT,  "not the same number of data_map_begin and data_map_end calls");

    return return_code;
#else
    CHECK(FALSE, NOT_IMPLEMENTED, "OpenMP 4.0 not supported; OpenMP TARGET feature not tested");
    return NOT_IMPLEMENTED;
#endif

}
