#include <omp.h>
#include <common.h>
#include <iostream>
#include <unistd.h>
#include <unistd.h>
#include <map>

using namespace std;
#define SMALL_NUM_THREADS 4
#define BIG_NUM_THREADS   20
ompt_get_parallel_team_size_t my_ompt_get_parallel_team_size;

void
init_test(ompt_function_lookup_t lookup) {
    my_ompt_get_parallel_team_size = (ompt_get_parallel_team_size_t)lookup("my_ompt_get_parallel_team_size"); 
    CHECK(my_ompt_get_parallel_team_size, NOT_IMPLEMENTED, "my_ompt_get_parallel_team_size is not implemented");
}

int
main()
{
   
    warmup();
    serialwork(1);
    int master_thread_id = ompt_get_thread_id();
   
    /* enable nested parallelism */
    omp_set_nested(1);

    #pragma omp parallel num_threads(SMALL_NUM_THREADS)
    {
        CHECK(my_ompt_get_parallel_team_size(0) == SMALL_NUM_THREADS, IMPLEMENTED_BUT_INCORRECT, \
              "Wrong team size");
        #pragma omp parallel num_threads(BIG_NUM_THREADS)
        {
            serialwork(1);
            CHECK(my_ompt_get_parallel_team_size(0) == SMALL_NUM_THREADS, IMPLEMENTED_BUT_INCORRECT, \
                  "Expect the same team size as before, not a bigger one");
            CHECK(my_ompt_get_parallel_team_size(1) == SMALL_NUM_THREADS, IMPLEMENTED_BUT_INCORRECT, \
                  "Expect the same team size as before, not a bigger one");
        }
    }

    /* when nested parallelism is disabled */
    omp_set_nested(0);
    #pragma omp parallel num_threads(SMALL_NUM_THREADS)
    {
        #pragma omp parallel num_threads(BIG_NUM_THREADS)
        {
            CHECK(my_ompt_get_parallel_team_size(0) == BIG_NUM_THREADS, IMPLEMENTED_BUT_INCORRECT, \
                  "Expect BIG_NUM_THREADS team size");
            CHECK(my_ompt_get_parallel_team_size(1) == SMALL_NUM_THREADS, IMPLEMENTED_BUT_INCORRECT, \
                  "Expect SMALL_NUM_THREADS team size");
        }
    }
    CHECK(my_ompt_get_parallel_team_size(1000) == -1, IMPLEMENTED_BUT_INCORRECT, "Need to return -1 when ancestor doesn't exist");
    return global_error_code;
}

