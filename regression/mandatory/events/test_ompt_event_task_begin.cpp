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
#define NUM_THREADS 4



//*****************************************************************************
// global variables
//*****************************************************************************

static std::map<ompt_task_id_t, ompt_task_id_t> task_id_to_parent_id_map;
static std::map<ompt_task_id_t, ompt_frame_t *> task_id_to_parent_frame_map;



//*****************************************************************************
// private operations 
//*****************************************************************************

static void 
on_ompt_event_task_begin(ompt_task_id_t parent_task_id,    
			 ompt_frame_t *parent_task_frame,  
			 ompt_task_id_t new_task_id,       
			 void *new_task_function)
{
  pthread_mutex_lock(&thread_mutex);

#if DEBUG
  printf("task_begin %lu (parent %lu)\n", new_task_id, parent_task_id);
#endif

#if 0
  CHECK(ompt_get_task_id(0) == parent_task_id,				\
	IMPLEMENTED_BUT_INCORRECT,					\
	"task begin callback doesn't execute in parent's context: "	\
	"current task id %lu, task begin parent_task_id %lu",		\
	ompt_get_task_id(0), parent_task_id);

  CHECK(ompt_get_task_frame(0) == parent_task_frame,			\
	IMPLEMENTED_BUT_INCORRECT,					\
	"task begin callback doesn't execute in parent's context: "	\
	"current task frame %p, task begin parent_task_frame %p",	\
	ompt_get_task_frame(0), parent_task_frame);
#endif

  CHECK(task_id_to_parent_id_map.count(new_task_id) == 0, \
	IMPLEMENTED_BUT_INCORRECT, "duplicate task ids");
  
  task_id_to_parent_id_map[new_task_id] = parent_task_id;
  task_id_to_parent_frame_map[new_task_id] = parent_task_frame;
  pthread_mutex_unlock(&thread_mutex);
}


static void 
print_level(int level, 
	    ompt_task_id_t level_id, 
	    ompt_task_id_t expected_parent_id,
	    ompt_frame_t *level_frame, 
	    ompt_frame_t *expected_parent_frame)
{
  ompt_task_id_t parent_id = ompt_get_task_id(1);
  ompt_frame_t *parent_frame = ompt_get_task_frame(1);
  printf("DEBUG: L%d task id = %lu. enclosing task id = %lu", 
	 level, level_id, parent_id); 
  if (parent_id != expected_parent_id) {
    printf(" ****expected %lu****\n", expected_parent_id);
  } else { 
    printf("\n");
  }
  printf("DEBUG: L%d frame = %p. enclosing frame = %p", 
	 level, level_frame, parent_frame); 
  if (parent_frame != expected_parent_frame) {
    printf(" ****expected %p****\n", expected_parent_frame);
  } else { 
    printf("\n");
  }
}



//*****************************************************************************
// interface operations 
//*****************************************************************************

void 
init_test(ompt_function_lookup_t lookup)
{
  if (!register_callback(ompt_event_task_begin, 
			 (ompt_callback_t) on_ompt_event_task_begin)) {
    CHECK(false, FATAL, "failed to register ompt_event_task_begin");
  }
}


void 
parallel_regions(ompt_task_id_t serial_id, ompt_frame_t *serial_frame)
{
  #pragma omp parallel num_threads(NUM_THREADS)
  {
   // #pragma omp master
    {
      ompt_task_id_t master_id = ompt_get_task_id(0);
      ompt_frame_t *master_frame = ompt_get_task_frame(0);
#if DEBUG
      print_level(1, master_id, serial_id, master_frame, serial_frame); 
#endif
      
      #pragma omp parallel num_threads(NUM_THREADS)
      {
        // #pragma omp master
        {
	ompt_task_id_t  level1_id = ompt_get_task_id(0);
	ompt_frame_t  *level1_frame = ompt_get_task_frame(0);
#if DEBUG
	print_level(2, level1_id, master_id, level1_frame, master_frame); 
#endif
	}
      }
    }
  }
}


int
regression_test(int argc, char** argv)
{
  omp_set_nested(1);
  ompt_task_id_t serial_id = ompt_get_task_id(0);
  ompt_frame_t *serial_frame = ompt_get_task_frame(0);


#if DEBUG
  print_level(0, serial_id, 0, serial_frame, 0); 
  printf("**** begin testing parallel regions ****\n");
#endif

  parallel_regions(serial_id, serial_frame);

#if DEBUG
  printf("**** done testing parallel regions ****\n");
#endif
  
#pragma omp parallel num_threads(NUM_THREADS)
  {
#pragma omp master
    {
      ompt_task_id_t master_id = ompt_get_task_id(0);
      ompt_frame_t *master_frame = ompt_get_task_frame(0);
#if DEBUG
      print_level(1, master_id, serial_id, master_frame, serial_frame); 
#endif
      
#pragma omp task
      {
	ompt_task_id_t  level1_id = ompt_get_task_id(0);
	ompt_frame_t  *level1_frame = ompt_get_task_frame(0);
#if DEBUG
	print_level(2, level1_id, master_id, level1_frame, master_frame); 
#endif
	
	pthread_mutex_lock(&thread_mutex);
	CHECK(ompt_get_task_id(1) ==					\
	      task_id_to_parent_id_map[level1_id],			\
	      IMPLEMENTED_BUT_INCORRECT,				\
	      "level 2 parent task id = %lu, expected %lu",		\
	      ompt_get_task_id(1), master_id);

	CHECK(ompt_get_task_frame(1) ==					\
	      task_id_to_parent_frame_map[level1_id],			\
	      IMPLEMENTED_BUT_INCORRECT,				\
	      "level 2 parent task frame = %p, expected %p",		\
	      ompt_get_task_frame(1), master_frame);
	pthread_mutex_unlock(&thread_mutex);
#pragma omp task
	{
	  serialwork(0);
	  ompt_task_id_t  level2_id = ompt_get_task_id(0);
	  ompt_frame_t  *level2_frame = ompt_get_task_frame(0);
#if DEBUG
	  print_level(3, level2_id, level1_id, level2_frame, level1_frame); 
#endif
	  pthread_mutex_lock(&thread_mutex);
	  CHECK(ompt_get_task_id(1) ==					\
		task_id_to_parent_id_map[level2_id],			\
		IMPLEMENTED_BUT_INCORRECT,				\
		"level 3 task parent task id = %lu, expected %lu",	\
		ompt_get_task_id(1), level1_id);
	  CHECK(ompt_get_task_frame(1) ==				\
		task_id_to_parent_frame_map[level2_id],			\
		IMPLEMENTED_BUT_INCORRECT,				\
		"level 3 parent task frame = %p, expected %p",		\
		ompt_get_task_frame(1), level1_frame);
	  pthread_mutex_unlock(&thread_mutex);
#pragma omp task
	  {
	    serialwork(1);
	  }
	}
      }
    }
  }
  return return_code;
}
