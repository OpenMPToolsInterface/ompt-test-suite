/*******************************************************************
 * This test will verify the correctness of retrieving frames
 * from OMPT library
 *******************************************************************/

#include <stdio.h>
#include <assert.h>

#include <execinfo.h>

#include <omp.h>
#include <ompt.h>


#define MAX_FRAMES 100

/*******************************************************************
 * ompt initialization
 *******************************************************************/

#define OMPT_FN_TYPE(fn) fn ## _t 
#define OMPT_FN_LOOKUP(lookup,fn) fn = (OMPT_FN_TYPE(fn)) lookup(#fn)
#define OMPT_FN_DECL(fn) OMPT_FN_TYPE(fn) fn

OMPT_FN_DECL(ompt_get_task_frame);
OMPT_FN_DECL(ompt_get_idle_frame);


int ompt_initialize(ompt_function_lookup_t lookup, const char *runtime_version, 
		    int ompt_version) 
{

  /* look up and bind OMPT API functions */
  OMPT_FN_LOOKUP(lookup,ompt_get_task_frame);
  OMPT_FN_LOOKUP(lookup,ompt_get_idle_frame);
  return 1;
}


/*******************************************************************
 * debugging purpose: print the current call stack, and check manually
 * with ompt_get_task_frame
 *
 *******************************************************************/
static void
get_backtrace()
{
 #ifdef __linux__
  void *buffer[MAX_FRAMES];
  int j;
  const int rank = omp_get_thread_num();

  const int nptrs = backtrace(buffer, MAX_FRAMES);
  char **strings = backtrace_symbols(buffer, nptrs);
  
  if (strings != NULL)
  {
    for(j=0; j<nptrs; j++) {
      printf("\t[%d] %d: %s\n", rank, j, strings[j]);
    }
  }
 #endif
  j = 0;
  while (1){
    if (__builtin_frame_address(1) != NULL) 
    {
      printf("\n[%d] frame %d: %p\n", rank, j, __builtin_frame_address(1));
      j++;
    } 
    else 
    {
	break;
    }
  } 
}

/*******************************************************************
 * get all frames from ompt_get_task_frame
 * 
 * @param frame: array of pointer to a frame
 * @param max_frames: maximum buffer in frame
 *
 * @return the number of frames returned by ompt_get_task_frame
 *******************************************************************/
static int
get_frames(ompt_frame_t *frame[], int max_frames)
{
  int depth = 0;
  ompt_frame_t *fr;
  const int rank = omp_get_thread_num();
  
  while (depth < max_frames) 
  {
    fr = ompt_get_task_frame(depth);

    if (fr) {
#ifdef OMPT_DEBUG
       printf("[%d] fr %d: %p -- %p\n ", rank, depth, fr->reenter_runtime_frame, fr->exit_runtime_frame);
#endif // OMPT_DEBUG
       frame[depth] = fr;
       depth++;
    } else {
	break;
    }
  } 

#ifdef OMPT_DEBUG
  fflush(stdout);
#endif
  return depth;
}


/*******************************************************************
 * main program
 *******************************************************************/

int 
main(int argc, char *argv[])
{
  // force OpenMP initialization
  omp_get_max_threads();
  
  void * idle_frame   = ompt_get_idle_frame();
  // idle_frame is not null at this stage with intel rtl
  // the TR doesn't say anything what will be the value of idle_frame
  // at this stage. So anything is possible ?
  //
  //assert( idle_frame == 0);

  ompt_frame_t *frame = ompt_get_task_frame(0);
  assert( frame->exit_runtime_frame == 0 && frame->reenter_runtime_frame == 0);

  #pragma omp parallel num_threads(2) private(idle_frame)
  {
    idle_frame = ompt_get_idle_frame();
    int thread = omp_get_thread_num();

    if (thread == 0)
    {
 	if (idle_frame != 0)
	{
	   fprintf(stderr, "[%d] idle_frame is not null: %p\n", thread, idle_frame);
	   assert( idle_frame == 0);
	}
    } else
    {
	assert( idle_frame != 0);
    }
    #pragma omp critical
    {
	ompt_frame_t *frames[MAX_FRAMES];
 	int depth = get_frames( frames, MAX_FRAMES);
	// ompt_get_task_frame should return at least 2 frames: 
	// one for entering the runtime, the other one for exiting
	// (see tr-2.pdf pp 34)
	assert(depth > 0);
	
	// tr-2 pp 34: the first frame has no reenter, but has exit
	assert(frames[0] != NULL);
	assert(frames[0]->reenter_runtime_frame == 0);
	assert(frames[0]->exit_runtime_frame  != 0);
	
	// tr2 pp 24: the second frame has reenter, but no exit
	assert(frames[1] != NULL);
	assert(frames[1]->reenter_runtime_frame  != 0);
	assert(frames[1]->exit_runtime_frame  == 0);
#ifdef OMPT_DEBUG
	get_backtrace();
#endif //OMPT_DEBUG
    }
  }
  exit(0);
}

