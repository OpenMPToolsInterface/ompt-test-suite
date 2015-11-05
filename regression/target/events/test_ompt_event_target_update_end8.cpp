//****************************************************************************
// macros 
//****************************************************************************

#define DEBUG 0


//****************************************************************************
// include test framework (registering events and general event tests)
//****************************************************************************

#include "target_update_end_framework.cpp"


//*************************************************************************
// test case 8: nested data regions and updates, do some random updates
//*************************************************************************

void update_end_test() {
    int *x = (int*) malloc(10 * sizeof(int));
    int *y = (int*) malloc(10 * sizeof(int));
    int a = 1;

    for (int i=0; i < 10; i++) {
        x[i] = i;
    }

    #pragma omp target data map(alloc: x[0:10])
    {
        // first level
        #pragma omp target update to(x[0:10])

        #pragma omp target data map(alloc: y[0:10])
        {
            // second level
            #pragma omp target update to(y[0:10])

            #pragma omp target data map(alloc: a)
            {
                // third level
                #pragma omp target update to(a)
                sleep(1);
                #pragma omp target update from(a)
            }
            #pragma omp target update from(y[0:10])
        }
        #pragma omp target update from(x[0:10])
    }

    CHECK(number_begin_events == 6, IMPLEMENTED_BUT_INCORRECT, "nested data regions and updates: number of target_task_begin events with task_type = ompt_target_task_update not as expected (expected %d, observed %d)", 6, number_begin_events);

}
