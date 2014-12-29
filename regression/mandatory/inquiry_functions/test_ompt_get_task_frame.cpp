#include <omp.h>
#include <common.h>
#include <iostream>
#include <unistd.h>
#include <unistd.h>

using namespace std;
#define NUM_THREADS 2
#define MAX_FRAMES 10

ompt_get_task_frame_t my_ompt_get_task_frame;
void
init_test(ompt_function_lookup_t lookup) 
{
    my_ompt_get_task_frame = (ompt_get_task_frame_t)lookup("ompt_get_task_frame"); 
    CHECK(my_ompt_get_task_frame, NOT_IMPLEMENTED, "ompt_get_task_frame is not implemented");
}


/* adapted from previous regression test */
int
get_frames(ompt_frame_t *frame[], int max_frames)
{
    int depth = 0;
    ompt_frame_t *fr;

    while (depth < max_frames) 
    {
        fr = my_ompt_get_task_frame(depth);

        if (fr) {
            frame[depth] = fr;
            depth++;
        } else {
            break;
        }
    } 
    return depth;
}

int
main(int argc, char **argv)
{
    register_segv_handler(argv);
    warmup();
    serialwork(1);
    ompt_frame_t * frames_begin[MAX_FRAMES];
    int depth = get_frames(frames_begin, MAX_FRAMES);
    CHECK(depth == 1, IMPLEMENTED_BUT_INCORRECT, "expect frame depth == 1");

    int master_thread_id = ompt_get_thread_id();
    omp_set_nested(1);
    #pragma omp parallel num_threads(NUM_THREADS)
    {
        serialwork(1); 
        ompt_frame_t *frames_level1[MAX_FRAMES];
        int depth = get_frames( frames_level1, MAX_FRAMES);

        /* 
         * my_ompt_get_task_frame should return at least 2 frames: 
         * one for entering the runtime, the other one for exiting
         * (see tr-2.pdf pp 34)
         */
        CHECK(depth >= 2, IMPLEMENTED_BUT_INCORRECT, "expect 2 frames after the initialization of the first parallel region");
        
        /*
         * the first frame(r2) has no reenter, but has exit(set by exiting
         * the runtime that initializes the first parallel region).
         */
        CHECK(frames_level1[0] != NULL, IMPLEMENTED_BUT_INCORRECT, "r2 should exist");
        CHECK(frames_level1[0]->reenter_runtime_frame == 0, IMPLEMENTED_BUT_INCORRECT, "r2 should have no reenter");
        CHECK(frames_level1[0]->exit_runtime_frame != 0, IMPLEMENTED_BUT_INCORRECT, "r2 should have exit");
        
        /*
         * the second frame(r1) has no exit reenter, but has reenter(set by entering
         * the runtime that initializes the first parallel region).
         */
        CHECK(frames_level1[1] != NULL, IMPLEMENTED_BUT_INCORRECT, "r1 should exist");
        CHECK(frames_level1[1]->reenter_runtime_frame != 0, IMPLEMENTED_BUT_INCORRECT, "r1 should have renter");
        CHECK(frames_level1[1]->exit_runtime_frame == 0, IMPLEMENTED_BUT_INCORRECT, "r1 should have no exit");

        /*
         * second frame(r1)'s reenter location should be less than the 
         * first frame(r2)'s exit location
         */
        CHECK(frames_level1[1]->reenter_runtime_frame <= frames_level1[0]->exit_runtime_frame, IMPLEMENTED_BUT_INCORRECT,
              "r1 should have a reenter address less than r2's exit address");
    
        #pragma omp parallel num_threads(NUM_THREADS)
        {
            serialwork(1);

            
            if (ompt_get_thread_id() != master_thread_id) {
                ompt_frame_t *frames_level2[MAX_FRAMES];
                int depth = get_frames( frames_level2, MAX_FRAMES);
                /*
                 * my_ompt_get_task_frame should return at least 3 frames in master thread:
                 * the previous r2, two new frames seperated by the runtime
                 * procedure that initializes the 2nd parallel region
                 */
                CHECK(depth >= 3, IMPLEMENTED_BUT_INCORRECT, "expect frame depth >= 3");
                
                /* the first(r4) frame has no reenter, but has exit */
                CHECK(frames_level2[0] != NULL, IMPLEMENTED_BUT_INCORRECT, "expect the last frame(r4) to be non-null");
                CHECK(frames_level2[0]->reenter_runtime_frame == 0, IMPLEMENTED_BUT_INCORRECT, "expect the last frame(r4) to have a null reenter pointer");
                CHECK(frames_level2[0]->exit_runtime_frame  != 0, IMPLEMENTED_BUT_INCORRECT, "expect the last frame(r4) to have an exit pointer");
    
                /* the second(r3) frame has both reenter and exit */
                CHECK(frames_level2[1] != NULL, IMPLEMENTED_BUT_INCORRECT, "expect the 2nd to last frame(r3) to be non-null");
                CHECK(frames_level2[1]->reenter_runtime_frame != 0, IMPLEMENTED_BUT_INCORRECT, "expect the 2nd to last frame(r3) has a reenter pointer");
                CHECK(frames_level2[1]->exit_runtime_frame != 0, IMPLEMENTED_BUT_INCORRECT, "expect the 2nd to last frame(r3) has a exit pointer");
                
                /* the third frame has no reenter, but has exit */
                CHECK(frames_level2[2] != NULL, IMPLEMENTED_BUT_INCORRECT, "expect the 3nd to last frame to be non-null");
                CHECK(frames_level2[2]->reenter_runtime_frame != 0, IMPLEMENTED_BUT_INCORRECT, "expect the 3rd to last frame to have a reenter pointer");
                CHECK(frames_level2[2]->exit_runtime_frame == 0, IMPLEMENTED_BUT_INCORRECT, "expect the 3rd to last frame has a null exit pointer"); 

                /* checking if the third frame is r1, the documention says the value of the structure is preserved */
                CHECK(frames_level2[2] == frames_level1[1], IMPLEMENTED_BUT_INCORRECT, "expect the 3rd frame to be same as r2");
            }
        }
    }

    ompt_frame_t * frames_end[MAX_FRAMES];
    depth = get_frames(frames_end, MAX_FRAMES);
    CHECK(depth == 1, IMPLEMENTED_BUT_INCORRECT, "expect frame depth == 1");
    return global_error_code;
}
