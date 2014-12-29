#include <omp.h>
#include <common.h>
#include <iostream>
#include <sstream>      
#include <map>

#define NUM_THREADS 1

using namespace std;

map<ompt_task_id_t, ompt_task_id_t> task_id_to_task_id_map;

ompt_get_task_id_t my_ompt_get_task_id;
void 
init_test(ompt_function_lookup_t lookup)
{
    my_ompt_get_task_id = (ompt_get_task_id_t)lookup("ompt_get_task_id");
    CHECK(my_ompt_get_task_id, NOT_IMPLEMENTED, "my_ompt_get_task_id is not implemented");
}

int
main(int argc, char** argv)
{
    register_segv_handler(argv);
    warmup();
    #pragma omp parallel num_threads(1)
    {
        #pragma omp master
        {
            ompt_task_id_t level0_task_id = my_ompt_get_task_id(0);
            #pragma omp task
            {
                serialwork(0);
                ompt_task_id_t  level1_task_id = my_ompt_get_task_id(0);
                CHECK(level0_task_id == my_ompt_get_task_id(1), IMPLEMENTED_BUT_INCORRECT, \
                      "level 1 parent task id is inconsistent");
                #pragma omp task
                {
                    serialwork(0);
                    ompt_task_id_t  level2_task_id = my_ompt_get_task_id(0);
                    CHECK(level1_task_id == my_ompt_get_task_id(1), IMPLEMENTED_BUT_INCORRECT, \
                      "level 2 parent task id is inconsistent");
                    CHECK(level0_task_id == my_ompt_get_task_id(2), IMPLEMENTED_BUT_INCORRECT, \
                      "level 2 grandparent task id is inconsistent");
                    #pragma omp task
                    {
                        CHECK(level2_task_id == my_ompt_get_task_id(1), IMPLEMENTED_BUT_INCORRECT, \
                            "level 3 parent task id is inconsistent");
                        CHECK(level1_task_id == my_ompt_get_task_id(2), IMPLEMENTED_BUT_INCORRECT, \
                            "level 3 parent task id is inconsistent");
                        serialwork(1);
                    }
                }
            }
        }
    }
    CHECK(my_ompt_get_task_id(1000) == 0, IMPLEMENTED_BUT_INCORRECT, "test at an invalid depth");
    return global_error_code;
}
