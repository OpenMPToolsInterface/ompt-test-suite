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
// macros
//*****************************************************************************

#define DEBUG 1


//*****************************************************************************
// global variables
//*****************************************************************************

int count = 0; // target_begin -> increased, target_end -> decreased
int number_begin_events = 0;

// save target_id and corresponding task_id for a target_begin 
ompt_target_id_t begin_target_id;
ompt_task_id_t begin_task_id;


//*****************************************************************************
// private operations
//*****************************************************************************

static void on_ompt_event_target_update_begin(ompt_task_id_t task_id,
                ompt_target_id_t target_id,
                ompt_target_device_id_t device_id,
                void* target_function) {
    pthread_mutex_unlock(&thread_mutex);

#if DEBUG
    printf("begin: task_id = %lld, target_id = %lld, device_id = %lld, target_function = %p\n",
        task_id, target_id, device_id, target_function);
#endif

    CHECK(task_id > 0, IMPLEMENTED_BUT_INCORRECT, "invalid task_id");
    CHECK(task_id == ompt_get_task_id(0), IMPLEMENTED_BUT_INCORRECT, "task_id not equal to ompt_get_task_id()");
    
    // save task_id and target_id for current thread
    begin_target_id = target_id;
    begin_task_id = task_id;

    count += 1;
    number_begin_events += 1;

    pthread_mutex_unlock(&thread_mutex);
}

static void on_ompt_event_target_update_end(ompt_task_id_t task_id,
                  ompt_target_id_t target_id) {
    pthread_mutex_lock(&thread_mutex);

#if DEBUG
    printf("end: task_id = %lld, target_id = %lld\n", task_id, target_id);
#endif

    CHECK(task_id > 0, IMPLEMENTED_BUT_INCORRECT, "invalid task_id");
    CHECK(task_id == ompt_get_task_id(0), IMPLEMENTED_BUT_INCORRECT, "task_id not equal to ompt_get_task_id()");

    // Check for correct target_id and task_id in target_end
    // (should be the same as in target_begin)
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

    // variable to be copied between host and device
    int a;

    // array to be copied between host and device
    int *x;

    //*************************************************************************
    // test case 1: copy variable from host to device
    //*************************************************************************
    
    // reset update begin event counter
    number_begin_events = 0;

    a = 0;
    #pragma omp target data map(alloc: a)
    {
        // modify value on host
        a = 1;

        // copy value to device
        #pragma omp target update to(a)

        bool device_ok = true;
        #pragma omp target
        {
            if (a != 1)
                device_ok = false;     
        }
        CHECK(device_ok, IMPLEMENTED_BUT_INCORRECT, "test case 1: copy variable from host to device not working correctly");
    }

    CHECK(number_begin_events == 1, IMPLEMENTED_BUT_INCORRECT, "test case 1 (copy variable from host to device): number of update_begin events not as expected (expected %d, observed %d)", 1, number_begin_events);

    //*************************************************************************
    // test case 2: copy variable from device to host
    //*************************************************************************

    // reset update begin event counter
    number_begin_events = 0;

    a = 0;
    #pragma omp target data map(alloc: a)
    {
        #pragma omp target
        {
            a = 1;
        }

        // copy value to host 
        #pragma omp target update from(a)


        CHECK(a == 1, IMPLEMENTED_BUT_INCORRECT, "test case 2: copy variable from device to host not working correctly");
    }

    CHECK(number_begin_events == 1, IMPLEMENTED_BUT_INCORRECT, "test case 2 (copy variable from device to host): number of update_begin events not as expected (expected %d, observed %d)", 1, number_begin_events);

    //*************************************************************************
    // test case 3: copy array from host to device
    //*************************************************************************

    // reset update begin event counter
    number_begin_events = 0;

    x = (int*) malloc(10 * sizeof(int));

    #pragma omp target data map(alloc: x[0:10])
    {
        // modify array on host
        for (int i=0; i < 10; i++) {
            x[i] = i;
        }
        
        // copy array to device
        #pragma omp target update to(x[0:10])

        bool device_ok = true;
        #pragma omp target
        { 
            for (int i=0; i < 10; i++) {
                if (x[i] != i)
                    device_ok = false;
            }
        }
        CHECK(device_ok, IMPLEMENTED_BUT_INCORRECT, "test case 3: copy array from host to device not working correctly");

    }

    CHECK(number_begin_events == 1, IMPLEMENTED_BUT_INCORRECT, "test case 3 (copy array from host to device): number of update_begin events not as expected (expected %d, observed %d)", 1, number_begin_events);

    //*************************************************************************
    // test case 4: copy array from device to host
    //*************************************************************************

    // reset update begin event counter
    number_begin_events = 0;

    x = (int*) malloc(10 * sizeof(int));

    #pragma omp target data map(alloc: x[0:10])
    {
        #pragma omp target
        { 
            // modify array on device 
            for (int i=0; i < 10; i++) {
                x[i] = i;
            }
           
        }
 
        // copy array to host 
        #pragma omp target update from(x[0:10])


        bool host_ok = true;
        for (int i=0; i < 10; i++) {
            if (x[i] != i)
                host_ok = false;
        }
        CHECK(host_ok, IMPLEMENTED_BUT_INCORRECT, "test case 4: copy array from device to host not working correctly");
    }

    CHECK(number_begin_events == 1, IMPLEMENTED_BUT_INCORRECT, "test case 4 (copy array from device to host): number of update_begin events not as expected (expected %d, observed %d)", 1, number_begin_events);

/*
    int a;
    // start value on host
    a = 1;
    
    // save old values for error checking
    int a_old_device, a_old_host;

    #pragma omp target data map(alloc: a, a_old_device)
    {
        #pragma omp target
        {
            // modify a on device
            a = 42;

            // save modified a on device
            a_old_device = a;
        }

        // save old a on host
        a_old_host = a;
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
*/
    CHECK(count == 0, IMPLEMENTED_BUT_INCORRECT,  "not the same number of target_update_begin and target_update_end calls");

    return return_code;
#else
    CHECK(FALSE, NOT_IMPLEMENTED, "OpenMP 4.0 not supported; OpenMP TARGET feature not tested");
    return NOT_IMPLEMENTED;
#endif

}
