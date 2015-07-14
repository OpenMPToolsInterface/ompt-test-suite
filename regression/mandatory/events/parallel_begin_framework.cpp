//*****************************************************************************
// system includes
//*****************************************************************************

#include <map>
#include <set>



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
// global variables
//*****************************************************************************

std::map<ompt_parallel_id_t, ompt_task_id_t> parallel_id_to_task_id_map;
std::map<ompt_parallel_id_t, ompt_frame_t *> parallel_id_to_task_frame_map;

std::set<ompt_parallel_id_t> parallel_id_set;
std::set<ompt_task_id_t> task_id_set;

ompt_task_id_t serial_task_id;
ompt_frame_t * serial_task_frame;
bool test_enclosing_context = false;

volatile int regions_encountered = 0;

int id_to_observe = -2;



//*****************************************************************************
// private operations
//*****************************************************************************

void dump_chain(int depth)
{
  ompt_task_id_t task_id = ompt_get_task_id(depth);

  if (task_id == id_to_observe) {
    ompt_task_id_t another_id = ompt_get_task_id(depth);
  }
  printf("level %d: task %" PRIu64 "\n", depth, task_id);
  if (task_id != 0) dump_chain(depth+1);
}


static void
on_ompt_event_parallel_begin
(ompt_task_id_t parent_task_id,    /* id of parent task            */
 ompt_frame_t *parent_task_frame,  /* frame data of parent task    */
 ompt_parallel_id_t parallel_id,   /* id of parallel region        */
 uint32_t requested_team_size,     /* number of threads in team    */
 void *parallel_function           /* pointer to outlined function */
 )
{
  CHECK(parent_task_id != 0,		\
	IMPLEMENTED_BUT_INCORRECT, \
        "parent task id = 0 in event_parallel_begin " \
        "parallel_id = %" PRIu64, parallel_id);

  CHECK(parallel_id != 0,		\
        IMPLEMENTED_BUT_INCORRECT,			  \
        "parallel region id = 0 in event_parallel_begin " \
        "parent_task_id = %" PRIu64, parent_task_id);

  CHECK(requested_team_size == NUM_THREADS,				\
	IMPLEMENTED_BUT_INCORRECT, "wrong requested team size");
  
  pthread_mutex_lock(&thread_mutex);
#if DEBUG
  printf("begin: parallel region id = %" PRIu64 ", parent_task_id = %" PRIu64 "\n", 
         parallel_id, parent_task_id);
  dump_chain(0);
#endif

  CHECK(parallel_id_to_task_id_map.count(parallel_id) == 0,		\
	IMPLEMENTED_BUT_INCORRECT, "duplicate parallel region id in task map");

  parallel_id_to_task_id_map[parallel_id] = parent_task_id;
  parallel_id_to_task_frame_map[parallel_id] = parent_task_frame;

  // check if this region id has been seen before
  std::set<ompt_parallel_id_t>::iterator iter  =
    parallel_id_set.find(parallel_id);
  
  CHECK(iter == parallel_id_set.end(), \
        IMPLEMENTED_BUT_INCORRECT, \
        "duplicate parallel region id %" PRIu64 " in region set", *iter);

  // record that this region id has been seen
  parallel_id_set.insert(parallel_id);

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
check_implicit_task_id(ompt_task_id_t task_id)
{
  pthread_mutex_lock(&thread_mutex);
  // check if this implicit task id has been seen before
  std::set<ompt_task_id_t>::iterator iter  =
    task_id_set.find(task_id);

  CHECK(iter == task_id_set.end(), \
        IMPLEMENTED_BUT_INCORRECT, \
        "duplicate implicit task id %" PRIu64, *iter);

  // record that this task id has been seen
  task_id_set.insert(task_id);
  pthread_mutex_unlock(&thread_mutex);
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

    check_implicit_task_id(task_id);

#if DEBUG
    {
      pthread_mutex_lock(&thread_mutex);
      printf("%*s enter region id %" PRIu64 ", task id = %" PRIu64 ", "
	     "parent task id %" PRIu64 " (threads = %d)\n", 
	     depth * 2, "", parallel_id, task_id, 
	     parent_task_id, omp_get_num_threads());
      pthread_mutex_unlock(&thread_mutex);
    }
#endif
    if(parent_task_id != parallel_id_to_task_id_map[parallel_id]) {
       ompt_task_id_t expected  = parallel_id_to_task_id_map[parallel_id];
       ompt_task_id_t parent_task_id2  = ompt_get_task_id(1);
    }
    
    pthread_mutex_lock(&thread_mutex);
    CHECK(parent_task_id == parallel_id_to_task_id_map[parallel_id],	\
	  IMPLEMENTED_BUT_INCORRECT,					\
	  "ompt_get_task_id(1) = %" PRIu64 " does not match task %" PRIu64 " that created " \
	  "region ompt_get_parallel_id(0)=%" PRIu64, parent_task_id, \
          parallel_id_to_task_id_map[parallel_id], parallel_id);
    
    CHECK(parent_frame == parallel_id_to_task_frame_map[parallel_id],	\
	  IMPLEMENTED_BUT_INCORRECT,					\
	  "ompt_get_task_frame(1) = %p does not match task that created " \
	  "region ompt_get_parallel_id(0)=%" PRIu64, parent_frame, parallel_id);
    pthread_mutex_unlock(&thread_mutex);
    
    fib_region_nesting(n - 1, depth + 1);
    fib_region_nesting(n - 2, depth + 1);
    
#if DEBUG
    {
      pthread_mutex_lock(&thread_mutex);
      printf("%*s exit region id %" PRIu64 ", task id = %" PRIu64 ", parent task id %" PRIu64 "\n", 
	     depth * 2, "", parallel_id, task_id, parent_task_id);
      pthread_mutex_unlock(&thread_mutex);
    }
#endif
    
  }
}


static void
fib_region_torture(int n, int iters)
{
  int iter, i;
  for(iter = 0; iter < iters; iter++) {
    for (i = 1; i < n; i++) {
      fib_region_nesting(i, 1);
    }
  }
}


static void 
simple_nested_region()
{

  #pragma omp atomic update
  regions_encountered += 1;

  #pragma omp parallel num_threads(NUM_THREADS)
  {
    #pragma omp barrier
    ompt_parallel_id_t level1_parallel_id = ompt_get_parallel_id(0);
    ompt_task_id_t     level1_task_id     = ompt_get_task_id(0);

    check_implicit_task_id(level1_task_id);

    CHECK(level1_parallel_id !=	0,				\
	  IMPLEMENTED_BUT_INCORRECT, "level1_parallel_id == 0");

    CHECK(level1_task_id != 0,				\
	  IMPLEMENTED_BUT_INCORRECT, "level1_task_id == 0");

    CHECK(ompt_get_task_id(1) ==					\
	  parallel_id_to_task_id_map[level1_parallel_id],		\
	  IMPLEMENTED_BUT_INCORRECT,					\
	  "level 1 parent task id does not match");
    CHECK(ompt_get_task_frame(1) ==					\
	  parallel_id_to_task_frame_map[level1_parallel_id],		\
	  IMPLEMENTED_BUT_INCORRECT,					\
	  "level 1 parent task frame does not match");
    
    #pragma omp master
    {
    #pragma omp atomic update
    regions_encountered += 1;
    #pragma omp parallel num_threads(NUM_THREADS)
    {
      serialwork(0);
      ompt_parallel_id_t level2_parallel_id = ompt_get_parallel_id(0);
      ompt_task_id_t     level2_task_id     = ompt_get_task_id(0);

      check_implicit_task_id(level2_task_id);

      CHECK(level2_parallel_id != 0,				\
	  IMPLEMENTED_BUT_INCORRECT, "level2_parallel_id == 0");

      CHECK(level2_task_id != 0,				\
	  IMPLEMENTED_BUT_INCORRECT, "level2_task_id == 0");


      CHECK(ompt_get_task_id(1) == parallel_id_to_task_id_map[level2_parallel_id], \
            IMPLEMENTED_BUT_INCORRECT, \
	    "level 2 parent task id does not match: " \
            "expected parent %" PRIu64 ", ompt_get_task_id(1) = %" PRIu64, \
            parallel_id_to_task_id_map[level2_parallel_id], ompt_get_task_id(1));

      CHECK(ompt_get_task_frame(1) == parallel_id_to_task_frame_map[level2_parallel_id], 
	    IMPLEMENTED_BUT_INCORRECT, "level 2 parent task frame does not match: " \
            "expected parent frame %p, ompt_get_task_frame(1) = %p", \
             parallel_id_to_task_frame_map[level2_parallel_id], ompt_get_task_frame(1));
      
      #pragma omp atomic update
      regions_encountered += 1;
      #pragma omp parallel num_threads(NUM_THREADS)
      {
        ompt_task_id_t     level3_task_id     = ompt_get_task_id(0);
        check_implicit_task_id(level3_task_id);
      }
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

  omp_set_nested(NESTED_VALUE);
    
#if DEBUG
  printf("*** nesting value = %d ***\n\n", NESTED_VALUE);
#endif
    
  test_enclosing_context = true;
  #pragma omp parallel num_threads(NUM_THREADS)
  {
    serialwork(0);
  }
  test_enclosing_context = false; 
  
  parallel_id_to_task_id_map.clear();
  parallel_id_to_task_frame_map.clear();

  simple_nested_region();

  fib_region_torture(FIB_N, FIB_ITERS);
  
  int begins = parallel_id_to_task_id_map.size();

  CHECK(begins == regions_encountered, IMPLEMENTED_BUT_INCORRECT,	\
	"parallel region begin doesn't match region entries (expected %d observed %d)", \
	regions_encountered, begins);

  return return_code;
}
