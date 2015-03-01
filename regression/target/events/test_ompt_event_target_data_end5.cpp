//****************************************************************************
// macros 
//****************************************************************************

#define DEBUG 0


//****************************************************************************
// include test framework (registering events and general event tests)
//****************************************************************************

#include "target_data_end_framework.cpp"


// ************************************************************************
// test case 5: nested data region
// ************************************************************************

void data_end_test() {
    int *x = (int*) malloc(10 * sizeof(int));
    int *y = (int*) malloc(100 * sizeof(int));
    int *z = (int*) malloc(1000 * sizeof(int));

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
    
    CHECK(number_begin_events == 3, IMPLEMENTED_BUT_INCORRECT, "number of data_begin events does not match with number of data regions (expected %d, observed %d)", 3, number_begin_events);
}
