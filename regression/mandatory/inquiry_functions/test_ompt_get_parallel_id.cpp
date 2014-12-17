#include <omp.h>
#include <iostream>
#include <map>
#include <common.h>

#define NUM_THREADS 4
using namespace std;


ompt_get_parallel_id_t my_ompt_get_parallel_id;

void init_test(ompt_function_lookup_t lookup) {
    my_ompt_get_parallel_id = (ompt_get_parallel_id_t)lookup("my_ompt_get_parallel_id"); 
    CHECK(my_ompt_get_parallel_id, NOT_IMPLEMENTED, "ompt_get_parallel_id is not implemented");
}


int main()
{
    warmup();
    CHECK(my_ompt_get_parallel_id(0) == 0, IMPLEMENTED_BUT_INCORRECT, "Outside a parallel region, my_ompt_get_parallel_id should return 0");
    
    #pragma omp parallel num_threads(NUM_THREADS)
    {
        CHECK(my_ompt_get_parallel_id(2) == 0, IMPLEMENTED_BUT_INCORRECT, "At level 1, expect an undefined grandpa id");
        CHECK(my_ompt_get_parallel_id(1) == 0, IMPLEMENTED_BUT_INCORRECT, "At level 1, expect an undefined parent id");
        ompt_parallel_id_t grandpa_parallel_id = my_ompt_get_parallel_id(0);
        CHECK(grandpa_parallel_id != 0, IMPLEMENTED_BUT_INCORRECT, "At level 1, expect a defined current id");

        #pragma omp parallel num_threads(NUM_THREADS)
        {
            CHECK(grandpa_parallel_id == my_ompt_get_parallel_id(1), IMPLEMENTED_BUT_INCORRECT,
                   "At level 2, expect to see a consistent grandpa id");
            ompt_parallel_id_t parent_parallel_id = my_ompt_get_parallel_id(0);
            CHECK(parent_parallel_id != 0, IMPLEMENTED_BUT_INCORRECT, "At level 2, expect a defined parent id");
            
            #pragma omp parallel num_threads(NUM_THREADS)
            {
                CHECK(grandpa_parallel_id == my_ompt_get_parallel_id(2), IMPLEMENTED_BUT_INCORRECT,
                       "At level 3, expect to see a consistent grandpa id");
                CHECK(parent_parallel_id == my_ompt_get_parallel_id(1), IMPLEMENTED_BUT_INCORRECT,
                       "At level 3, expect to see a consistent parent id");
                ompt_parallel_id_t parallel_id = my_ompt_get_parallel_id(0);
                CHECK(parallel_id != 0, IMPLEMENTED_BUT_INCORRECT,
                       "At level 3, expect to see a defined current id");
                CHECK(parallel_id != parent_parallel_id && parent_parallel_id != grandpa_parallel_id &&
                       parallel_id != grandpa_parallel_id
                       , IMPLEMENTED_BUT_INCORRECT, "At level 3, expect to see a different id at each level");
            }
        }
    }
    
    CHECK(my_ompt_get_parallel_id(0) == 0, IMPLEMENTED_BUT_INCORRECT, "Outside a parallel region, my_ompt_get_parallel_id should return 0");
    return global_error_code;
}
