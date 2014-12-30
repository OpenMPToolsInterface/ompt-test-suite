#include <omp.h>
#include <common.h>
#include <iostream>
#include <sstream>      

#define NUM_THREADS 4

using namespace std;

void on_ompt_event_runtime_shutdown(uint64_t command,  uint64_t modifier)
{
    exit(CORRECT);
}

void 
init_test(ompt_function_lookup_t lookup)
{
    global_error_code = NOT_IMPLEMENTED;
    if (!register_callback(ompt_event_runtime_shutdown, (ompt_callback_t) on_ompt_event_runtime_shutdown)) {
        CHECK(FALSE, FATAL, "failed to register ompt_event_runtime_shutdown");
    }
}

void work()
{
    #pragma omp parallel num_threads(NUM_THREADS)
    {
        #pragma omp parallel num_threads(NUM_THREADS) 
        {
          serialwork(1);
        }
    }
}

int
main(int argc, char** argv)
{
    register_segv_handler(argv);
    warmup();
    work();
    usleep(3000000); /* sleep 3s */
    return global_error_code;
}
