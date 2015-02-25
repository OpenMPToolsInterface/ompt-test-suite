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
// private operations
//*****************************************************************************

static void on_ompt_event_target_data_begin(ompt_task_id_t task_id,
                ompt_target_id_t target_id,
                ompt_target_device_id_t device_id,
                void* target_function) {
    pthread_mutex_lock(&thread_mutex);

#if DEBUG
    printf("begin: task_id = %lld, target_id = %lld, device_id = %lld, target_function = %p\n",
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
    printf("end: task_id = %lld, target_id = %lld\n",
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
    int a;
    int *x, *y, *z;


#if (TEST == 1)
    // ************************************************************************
    // test case 1: empty data region
    // ************************************************************************

    // reset counter
    number_begin_events = 0;

    #pragma omp target data
    {
        sleep(1);
    }

    CHECK(number_begin_events == 1, IMPLEMENTED_BUT_INCORRECT, "test case 1 (empty data region): number of data_begin events does not match with number of data regions (expected %d, observed %d)", 1, number_begin_events);


#elif (TEST == 2) 
    // ************************************************************************
    // test case 2: alloc variable 
    // ************************************************************************

    // reset counter
    number_begin_events = 0;


    a = 1;
    int b = 2;
    #pragma omp target data map(alloc: a)
    {   
        sleep(1);
    } 

    CHECK(number_begin_events == 1, IMPLEMENTED_BUT_INCORRECT, "test case 2 (alloc variable): number of data_begin events does not match with number of data regions (expected %d, observed %d)", 1, number_begin_events);


#elif (TEST == 3)
    // ************************************************************************
    // test case 3: copy (tofrom) array
    // ************************************************************************

    // reset counter
    number_begin_events = 0;

    y = (int*) malloc(10 * sizeof(int));

    #pragma omp target data map(tofrom: y[0:10])
    {
        sleep(1);
    }
    
    CHECK(number_begin_events == 1, IMPLEMENTED_BUT_INCORRECT, "test case 3 (tofrom: array): number of data_begin events does not match with number of data regions (expected %d, observed %d)", 1, number_begin_events);


#elif (TEST == 4)
    // ***********************************************************************
    // test case 4: copy (tofrom) array and variable together
    // ***********************************************************************
    
    // reset counter
    number_begin_events = 0;

    z = (int*) malloc(10 * sizeof(int));
    a = 1;

    #pragma omp target data map(tofrom: z[0:10], a)
    {
        sleep(1);
    }

    CHECK(number_begin_events == 1, IMPLEMENTED_BUT_INCORRECT, "test case 4 (tofrom: array and variable): number of data_begin events does not match with number of data regions (expected %d, observed %d)", 1, number_begin_events);


#elif (TEST == 5)
    // ************************************************************************
    // test case 5: nested data region
    // ************************************************************************

    // reset counter
    number_begin_events = 0;

    x = (int*) malloc(10 * sizeof(int));
    y = (int*) malloc(100 * sizeof(int));
    z = (int*) malloc(1000 * sizeof(int));

    #pragma omp target data map(tofrom: x[0:10])
    {
        #pragma omp target data map(tofrom: y[0:100])
        {
            #pragma omp target data map(tofrom: z[0:1000])
            {
                sleep(1);
            }
        }
    }
    
    CHECK(number_begin_events == 3, IMPLEMENTED_BUT_INCORRECT, "test case 5 (nested arrays): number of data_begin events does not match with number of data regions (expected %d, observed %d)", 3, number_begin_events);


#elif (TEST == 6)
    // ************************************************************************
    // test case 6: multiple arrays 
    // ************************************************************************

    // reset counter
    number_begin_events = 0;


    x = (int*) malloc(10 * sizeof(int));
    y = (int*) malloc(100 * sizeof(int));
    z = (int*) malloc(1000 * sizeof(int));

    #pragma omp target data map(tofrom: x[0:10], y[0:100], z[0:1000])
    {
        sleep(1);
    }
    
    CHECK(number_begin_events == 1, IMPLEMENTED_BUT_INCORRECT, "test case 6 (multiple arrays): number of data_begin events does not match with number of data regions (expected %d, observed %d)", 1, number_begin_events);

#endif

    CHECK(count == 0, IMPLEMENTED_BUT_INCORRECT,  "not the same number of target_data_begin and target_data_end calls (count = %d)", count);

    return return_code;
#else
    CHECK(FALSE, NOT_IMPLEMENTED, "OpenMP 4.0 not supported; OpenMP TARGET feature not tested");
    return NOT_IMPLEMENTED;
#endif

}
