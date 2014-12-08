#include <omp.h>
#include <common.h>
#include <iostream>
#include <assert.h>
#include <unistd.h>
#include <unistd.h>

using namespace std;
#define NUM_THREADS 4
#define MAX_FRAMES 5

void
init_test(ompt_function_lookup_t lookup) {
    ASSERT(ompt_get_task_frame, NOT_IMPLEMENTED, "ompt_get_task_frame is not implemented");
}

int
get_frames(ompt_frame_t *frame[], int max_frames)
{
    int depth = 0;
    ompt_frame_t *fr;

    while (depth < max_frames) 
    {
        fr = ompt_get_task_frame(depth);

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
main()
{
    warmup();
    serialwork(1);
    #pragma omp parallel num_threads(NUM_THREADS)
    {
        serialwork(1); 
        
        #pragma omp master
        {
            ompt_frame_t *frames[MAX_FRAMES];
            int depth = get_frames( frames, MAX_FRAMES);
            // ompt_get_task_frame should return at least 2 frames: 
            // one for entering the runtime, the other one for exiting
            // (see tr-2.pdf pp 34)
            assert(depth > 1);
            
            // tr-2 pp 34: the first frame has no reenter, but has exit
            assert(frames[0] != NULL);
            assert(frames[0]->reenter_runtime_frame == 0);
            assert(frames[0]->exit_runtime_frame  != 0);
            
            // tr2 pp 24: the second frame has reenter, but no exit
            assert(frames[1] != NULL);
            assert(frames[1]->reenter_runtime_frame  != 0);
            assert(frames[1]->exit_runtime_frame  == 0); 
        } 

        #pragma omp parallel num_threads(NUM_THREADS)
        {
            serialwork(1);

            #pragma omp master
            {
                ompt_frame_t *frames[MAX_FRAMES];
                int depth = get_frames( frames, MAX_FRAMES);
                // ompt_get_task_frame should return at least 3 frames: 
                // one for entering the runtime, another one for exiting,
                // and the last one for entering again
                // (see tr-2.pdf pp 34)
                assert(depth > 2);
                //
                // tr-2 pp 34: the first frame has no reenter, but has exit
                assert(frames[0] != NULL);
                assert(frames[0]->reenter_runtime_frame == 0);
                assert(frames[0]->exit_runtime_frame  != 0);
    
                // tr2 pp 24: the second frame has both reenter and exit
                assert(frames[1] != NULL);
                assert(frames[1]->reenter_runtime_frame  != 0);
                assert(frames[1]->exit_runtime_frame  != 0);
                //
                // tr-2 pp 34: the first frame has no reenter, but has exit
                assert(frames[2] != NULL);
                assert(frames[2]->reenter_runtime_frame != 0);
                assert(frames[2]->exit_runtime_frame  == 0);
            }
        }
    }
}
