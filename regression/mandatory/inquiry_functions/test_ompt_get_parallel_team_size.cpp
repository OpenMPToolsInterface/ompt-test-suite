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
    my_ompt_get_parallel_team_size = (ompt_get_parallel_team_size_t)lookup("ompt_get_parallel_team_size"); 
    CHECK(my_ompt_get_parallel_team_size, FATAL, "failed to register ompt_get_parallel_team_size");
}

int
main(int argc, char **argv)
{
    register_segv_handler(argv);
    warmup();
    serialwork(1);
    int master_thread_id = ompt_get_thread_id();
   
    /* enable nested parallelism */
    omp_set_nested(1);

    #pragma omp parallel num_threads(SMALL_NUM_THREADS)
    {
        int team_size_0 = omp_get_team_size(omp_get_level());
        CHECK(my_ompt_get_parallel_team_size(0) == team_size_0, IMPLEMENTED_BUT_INCORRECT, \
              "wrong team size");
        #pragma omp parallel num_threads(BIG_NUM_THREADS)
        {
            int team_size_1 = omp_get_team_size(omp_get_level());
            serialwork(1);
            CHECK(my_ompt_get_parallel_team_size(0) == team_size_1, IMPLEMENTED_BUT_INCORRECT, \
                  "expect the same team size as before, not a bigger one");
            CHECK(my_ompt_get_parallel_team_size(1) == team_size_0, IMPLEMENTED_BUT_INCORRECT, \
                  "expect the same team size as before, not a bigger one");
        }
    }

    /* when nested parallelism is disabled */
    omp_set_nested(0);
    #pragma omp parallel num_threads(SMALL_NUM_THREADS)
    {
        int team_size_0 = omp_get_team_size(omp_get_level());
        #pragma omp parallel num_threads(BIG_NUM_THREADS)
        {
            int team_size_1 = omp_get_team_size(omp_get_level());
            CHECK(my_ompt_get_parallel_team_size(0) == team_size_1, IMPLEMENTED_BUT_INCORRECT, \
                  "expect BIG_NUM_THREADS team size");
            CHECK(my_ompt_get_parallel_team_size(1) == team_size_0, IMPLEMENTED_BUT_INCORRECT, \
                  "expect SMALL_NUM_THREADS team size");
        }
    }
    CHECK(my_ompt_get_parallel_team_size(1000) == -1, IMPLEMENTED_BUT_INCORRECT, \
          "need to return -1 when ancestor doesn't exist");
    return global_error_code;
}

