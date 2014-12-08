#include <omp.h>
#include <iostream>
#include <map>
#include <assert.h>
#include <common.h>

using namespace std;
void init_test(ompt_function_lookup_t lookup) {
    ASSERT(ompt_get_parallel_id, NOT_IMPLEMENTED, "ompt_get_parallel_id is not implemented");
}


int main()
{
    ompt_parallel_id_t parallel_id, parent_parallel_id, grandpa_parallel_id;
    parallel_id = ompt_get_parallel_id(0);
    ASSERT(parallel_id == 0, IMPLEMENTED_BUT_INCORRECT, "Outside a parallel region, ompt_get_parallel_id should return 0");
    
    #pragma omp parallel num_threads(NUM_THREADS)
    {
        ASSERT(ompt_get_parallel_id(2) == 0, IMPLEMENTED_BUT_INCORRECT, "At level 1, expect an undefined grandpa id");
        ASSERT(ompt_get_parallel_id(1) == 0, IMPLEMENTED_BUT_INCORRECT, "At level 1, expect an undefined parent id");
        grandpa_parallel_id = ompt_get_parallel_id(0);
        ASSERT(grandpa_parallel_id != 0, IMPLEMENTED_BUT_INCORRECT, "At level 1, expect a defined current id");

        #pragma omp parallel num_threads(NUM_THREADS)
        {
            ASSERT(grandpa_parallel_id == ompt_get_parallel_id(1), IMPLEMENTED_BUT_INCORRECT,
                   "At level 2, expect to see a consistent grandpa id");
            parent_parallel_id = ompt_get_parallel_id(0);
            ASSERT(parent_parallel_id != 0, IMPLEMENTED_BUT_INCORRECT, "At level 2, expect a defined parent id");
            
            #pragma omp parallel num_threads(NUM_THREADS)
            {
                ASSERT(grandpa_parallel_id == ompt_get_parallel_id(2), IMPLEMENTED_BUT_INCORRECT,
                       "At level 3, expect to see a consistent grandpa id");
                ASSERT(parent_parallel_id == ompt_get_parallel_id(1), IMPLEMENTED_BUT_INCORRECT,
                       "At level 3, expect to see a consistent parent id");
                parallel_id = ompt_get_parallel_id(0);
                ASSERT(parallel_id != 0, IMPLEMENTED_BUT_INCORRECT,
                       "At level 3, expect to see a defined current id");
                ASSERT(parallel_id != parent_parallel_id && parent_parallel_id != grandpa_parallel_id &&
                       parallel_id != grandpa_parallel_id
                       , IMPLEMENTED_BUT_INCORRECT, "At level 3, expect to see a different id at each level");
            }
        }
    }
    
    parallel_id = ompt_get_parallel_id(0);
    ASSERT(parallel_id == 0, IMPLEMENTED_BUT_INCORRECT, "Outside a parallel region, ompt_get_parallel_id should return 0");
    return CORRECT;
}
