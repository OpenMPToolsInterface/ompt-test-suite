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

#define DEBUG 1


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
    printf("task_id = %lld, target_id = %lld, device_id = %lld, target_function = %p\n",
        task_id, target_id, device_id, target_function);
#endif

    CHECK(task_id > 0, IMPLEMENTED_BUT_INCORRECT, "invalid task_id");
    CHECK(task_id == ompt_get_task_id(0), IMPLEMENTED_BUT_INCORRECT, "task_id not equal to ompt_get_task_id()");    

    number_begin_events += 1;
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
    // task_id=0 workaround
    // TODO: fix in OMPT implementation
    #pragma omp parallel    
    {
    }

    // ************************************************************************
    // test case 1: empty data-region
    // ************************************************************************

    // reset counter
    number_begin_events = 0;

    #pragma omp target data
    {
        sleep(1);
    }
    
    CHECK(number_begin_events == 1, IMPLEMENTED_BUT_INCORRECT, "test case 1: number of data_begin events does not match with number of data regions (expected %d, observed %d)", 1, number_begin_events);


    // ************************************************************************
    // test case 2: alloc an array without passing a size
    // ************************************************************************

    // reset counter
    number_begin_events = 0;

    int *x = (int*) malloc(10 * sizeof(int));

    #pragma omp target data map(alloc: x)
    {   
        sleep(1);
    } 

    CHECK(number_begin_events == 1, IMPLEMENTED_BUT_INCORRECT, "test case 2: number of data_begin events does not match with number of data regions (expected %d, observed %d)", 1, number_begin_events);


    // ************************************************************************
    // test case 3: copy (tofrom) array
    // ************************************************************************

    // reset counter
    number_begin_events = 0;

    int *y = (int*) malloc(10 * sizeof(int));

    #pragma omp target data map(tofrom: y[0:10])
    {
        sleep(1);
    }
    
    CHECK(number_begin_events == 1, IMPLEMENTED_BUT_INCORRECT, "test case 3: number of data_begin events does not match with number of data regions (expected %d, observed %d)", 1, number_begin_events);


    // ***********************************************************************
    // test case 4: copy (tofrom) array and variable together
    // ***********************************************************************
    
    // reset counter
    number_begin_events = 0;

    int *z = (int*) malloc(10 * sizeof(int));
    int a = 1;

    #pragma omp target data map(tofrom: z[0:10], a)
    {
        sleep(1);
    }

    CHECK(number_begin_events == 1, IMPLEMENTED_BUT_INCORRECT, "test case 4: number of data_begin events does not match with number of data regions (expected %d, observed %d)", 1, number_begin_events);


    // ************************************************************************
    // test case 5: nested data region
    // ************************************************************************

    // reset counter
    number_begin_events = 0;

    int *x1 = (int*) malloc(10 * sizeof(int));
    int *x2 = (int*) malloc(100 * sizeof(int));
    int *x3 = (int*) malloc(1000 * sizeof(int));

    #pragma omp target data map(tofrom: x1[0:10])
    {
        #pragma omp target data map(tofrom: x2[0:100])
        {
            #pragma omp target data map(tofrom: x3[0:1000])
            {
                sleep(1);
            }
        }
    }
    
    CHECK(number_begin_events == 3, IMPLEMENTED_BUT_INCORRECT, "test case 5: number of data_begin events does not match with number of data regions (expected %d, observed %d)", 1, number_begin_events);


    return return_code;
#else
    CHECK(FALSE, NOT_IMPLEMENTED, "OpenMP 4.0 not supported; OpenMP TARGET feature not tested");
    return NOT_IMPLEMENTED;
#endif

}
