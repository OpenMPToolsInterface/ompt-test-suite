//*****************************************************************************
// system includes
//*****************************************************************************

#include <map>



//*****************************************************************************
// OpenMP runtime includes
//*****************************************************************************

#include <omp.h>



//*****************************************************************************
// regression harness includes
//*****************************************************************************

#include <ompt-regression.h>
#include <ompt-initialize.h>



//*****************************************************************************
// macros
//*****************************************************************************

#define DEBUG 0
#define NUM_THREADS 2



//*****************************************************************************
// global variables
//*****************************************************************************

std::map<ompt_parallel_id_t, ompt_task_id_t> parallel_id_to_task_id_map;
std::map<ompt_parallel_id_t, ompt_frame_t *> parallel_id_to_task_frame_map;

ompt_task_id_t serial_task_id;
ompt_frame_t * serial_task_frame;
bool test_enclosing_context = false;

volatile int regions_encountered = 0;



//*****************************************************************************
// private operations
//*****************************************************************************

static void
on_ompt_event_parallel_begin
(ompt_task_id_t parent_task_id,    /* id of parent task            */
 ompt_frame_t *parent_task_frame,  /* frame data of parent task    */
 ompt_parallel_id_t parallel_id,   /* id of parallel region        */
 uint32_t requested_team_size,     /* number of threads in team    */
 void *parallel_function           /* pointer to outlined function */
 )
{
  CHECK(parallel_id_to_task_id_map.count(parallel_id) == 0,		\
	IMPLEMENTED_BUT_INCORRECT, "duplicated parallel region ids");
  CHECK(requested_team_size == NUM_THREADS,				\
	IMPLEMENTED_BUT_INCORRECT, "wrong requested team size");
  
  pthread_mutex_lock(&thread_mutex);
  parallel_id_to_task_id_map[parallel_id] = parent_task_id;
  parallel_id_to_task_frame_map[parallel_id] = parent_task_frame;
  pthread_mutex_unlock(&thread_mutex);
  
  if (test_enclosing_context) {
    CHECK(ompt_get_task_id(0) == serial_task_id,			\
	  IMPLEMENTED_BUT_INCORRECT,					\
	  "parallel begin callback doesn't execute in parent's context");
    
    CHECK(ompt_get_task_frame(0) == serial_task_frame,			\
	  IMPLEMENTED_BUT_INCORRECT,					\
	  "parallel begin callback doesn't execute in parent's context");
  }
}


static void 
fib_region_nesting(int n, int depth)
{
  if (n < 1) return;
  
  #pragma omp atomic update
  regions_encountered += 1;
  
  
  #pragma omp parallel num_threads(NUM_THREADS)
  {
    ompt_parallel_id_t parallel_id     = ompt_get_parallel_id(0);
    ompt_task_id_t     task_id         = ompt_get_task_id(0);
    ompt_task_id_t     parent_task_id  = ompt_get_task_id(1);
    ompt_frame_t      *parent_frame    = ompt_get_task_frame(1);
    
#if DEBUG
    {
      pthread_mutex_lock(&thread_mutex);
      printf("%*s enter region id %lld, task id = %lld, "
	     "parent task id %lld (threads = %d)\n", 
	     depth * 2, "", parallel_id, task_id, 
	     parent_task_id, omp_get_num_threads());
      pthread_mutex_unlock(&thread_mutex);
    }
#endif
    
    pthread_mutex_lock(&thread_mutex);
    CHECK(parent_task_id == parallel_id_to_task_id_map[parallel_id],	\
	  IMPLEMENTED_BUT_INCORRECT,					\
	  "ompt_get_task_id(1) = %lld does not match task that created " \
	  "region ompt_get_parallel_id(0)=%lld", parent_task_id, parallel_id);
    
    CHECK(parent_frame == parallel_id_to_task_frame_map[parallel_id],	\
	  IMPLEMENTED_BUT_INCORRECT,					\
	  "ompt_get_task_frame(1) = %p does not match task that created " \
	  "region ompt_get_parallel_id(0)=%lld", parent_frame, parallel_id);
    pthread_mutex_unlock(&thread_mutex);
    
    fib_region_nesting(n - 1, depth + 1);
    fib_region_nesting(n - 2, depth + 1);
    
#if DEBUG
    {
      pthread_mutex_lock(&thread_mutex);
      printf("%*s exit region id %lld, task id = %lld, parent task id %lld\n", 
	     depth * 2, "", parallel_id, task_id, parent_task_id);
      pthread_mutex_unlock(&thread_mutex);
    }
#endif
    
  }
}


static void
fib_region_torture(int n, int iters, int nlevels)
{
   int iter, level, i;
   for (level = 0; level < nlevels; level++) {
      omp_set_nested(level);
      for(iter = 0; iter < iters; iter++) {
         for (i = 1; i < n; i++) {
           fib_region_nesting(i, 1);
         }
      }
   }
}



//*****************************************************************************
// interface operations
//*****************************************************************************

void 
init_test(ompt_function_lookup_t lookup)
{
  if (!register_callback(ompt_event_parallel_begin, 
			 (ompt_callback_t) on_ompt_event_parallel_begin)) {
    CHECK(false, FATAL, "failed to register ompt_event_parallel_begin");
  }
}


int
regression_test(int argc, char** argv)
{
  /* test whether callback executes in parent enclosing context */
  serial_task_id = ompt_get_task_id(0);
  serial_task_frame = ompt_get_task_frame(0);
  
  test_enclosing_context = true;
  #pragma omp parallel num_threads(NUM_THREADS)
  {
    serialwork(0);
  }
  test_enclosing_context = false; 
  
  parallel_id_to_task_id_map.clear();
  parallel_id_to_task_frame_map.clear();
  
  omp_set_nested(3);
  regions_encountered += 1;
  test_enclosing_context = true;
  #pragma omp parallel num_threads(NUM_THREADS)
  {
    test_enclosing_context = false;
    #pragma omp barrier
    serialwork(0);
    ompt_parallel_id_t level1_parallel_id = ompt_get_parallel_id(0);
    ompt_task_id_t   level1_task_id = ompt_get_task_id(0);
    CHECK(ompt_get_task_id(1) ==					\
	  parallel_id_to_task_id_map[level1_parallel_id],		\
	  IMPLEMENTED_BUT_INCORRECT,					\
	  "level 1 parent task id does not match");
    CHECK(ompt_get_task_frame(1) ==					\
	  parallel_id_to_task_frame_map[level1_parallel_id],		\
	  IMPLEMENTED_BUT_INCORRECT,					\
	  "level 1 parent task frame does not match");
    
    #pragma omp atomic update
    regions_encountered += 1;
    #pragma omp parallel num_threads(NUM_THREADS)
    {
      serialwork(0);
      ompt_parallel_id_t level2_parallel_id = ompt_get_parallel_id(0);
      CHECK(ompt_get_task_id(1) == parallel_id_to_task_id_map[level2_parallel_id], IMPLEMENTED_BUT_INCORRECT, 
	    "level 2 parent task id does not match");
      CHECK(ompt_get_task_frame(1) == parallel_id_to_task_frame_map[level2_parallel_id], 
	    IMPLEMENTED_BUT_INCORRECT, "level 2 parent task frame does not match");
      
      #pragma omp atomic update
      regions_encountered += 1;
      #pragma omp parallel num_threads(NUM_THREADS)
      {
	serialwork(0);
      }
    }
    #pragma omp barrier
    test_enclosing_context = true;
  }
  test_enclosing_context = false;
  
  fib_region_torture(4, 2, 4);
  
  int begins = parallel_id_to_task_id_map.size();
  CHECK(begins == regions_encountered, IMPLEMENTED_BUT_INCORRECT,	\
	"parallel region begin doesn't match region entries (expected %d observed %d)", \
	regions_encountered, begins);
  return return_code;
}
