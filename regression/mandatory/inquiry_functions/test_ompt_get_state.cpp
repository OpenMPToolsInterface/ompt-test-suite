//*****************************************************************************
// system includes
//*****************************************************************************

#include <map>
#include <vector>

#include <signal.h>



//*****************************************************************************
// OpenMP runtime includes
//*****************************************************************************

#include <omp.h>



//*****************************************************************************
// regression harness includes
//*****************************************************************************

#include <ompt-regression.h>
#include <ompt-initialize.h>

#include <states.h>
#include <timer.h>



//*****************************************************************************
// macros
//*****************************************************************************

#define NUM_THREADS 4
#define VECTOR_LENGTH 20000000



//*****************************************************************************
// global data
//*****************************************************************************

static bool test_implicit_barrier = false;
static bool monitor_explicit_barrier = false;
static bool test_taskwait = false;
static bool test_taskgroup = false;

std::vector<ompt_state_t> observed_states;

ompt_get_state_t my_ompt_get_state;

bool timer_signal_blocked = false;

static void 
collect_trace(ompt_state_t state)
{
    if (!timer_signal_blocked) {
        pthread_mutex_lock(&thread_mutex);
        observed_states.push_back(state);
        pthread_mutex_unlock(&thread_mutex);
    }
}

static void
trace_collector_callback(int sig, siginfo_t *si, void *uc)
{
  ompt_wait_id_t currWait;
  ompt_state_t current_state = my_ompt_get_state(&currWait);
  
  if (observed_states.size() == observed_states.capacity()) {
    printf("observed_states state vector capacity exhausted; "	\
	   "can't log any more states in signal handler");
  }
  
  observed_states.push_back(current_state);
}


static int 
signal_block()
{
  timer_signal_blocked = true;
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGRTMIN);
  if (sigprocmask(SIG_SETMASK, &mask, NULL) == -1) {
    return -1;
  }
  return 0;
}


static int 
signal_unblock()
{
 timer_signal_blocked = false;
 sigset_t mask;
 sigemptyset(&mask);
 sigaddset(&mask, SIGRTMIN);
 if (sigprocmask(SIG_UNBLOCK, &mask, NULL) == -1) {
   return -1;
 }
 return 0;
}


static int 
monitor_epilogue()
{
  return signal_block();
}


static int
monitor_prologue()
{
  pthread_mutex_lock(&thread_mutex);
  observed_states.clear();
  pthread_mutex_unlock(&thread_mutex);

  if (signal_unblock() == -1) {
    return -1;
  }
  return 0;
}



//*****************************************************************************
// interface operations
//*****************************************************************************

void
init_test(ompt_function_lookup_t lookup) 
{
  my_ompt_get_state = (ompt_get_state_t)lookup("ompt_get_state"); 
  CHECK(my_ompt_get_state, NOT_IMPLEMENTED,	\
	"failed to register ompt_get_state");
  quit_on_init_failure();
}


int
regression_test(int argc, char **argv)
{
  observed_states.reserve(VECTOR_LENGTH);
  
  /* set up a timer */
  Timer timer; 
  init_timer(&timer);
  register_timer_callback(&timer, trace_collector_callback);

  signal_unblock(); // warm up code
  signal_block();   // block before starting timer

  start_timer(&timer, 10000);
  
  //--------------------------------------------------------------------------
  // TEST 0: Outside parallel region, expect to see ompt_state_work_serial 
  //--------------------------------------------------------------------------
  monitor_prologue();
  serialwork(2);
  monitor_epilogue();
  
  CHECK(check_states(observed_states, "(ompt_state_work_serial)+"),	\
	IMPLEMENTED_BUT_INCORRECT,					\
	"expect state serial outside parallel regions; observed '%s'",	\
	state_string(observed_states).c_str());
  

  //--------------------------------------------------------------------------
  // TEST1: Inside parallel region, expect to see ompt_state_work_parallel
  //--------------------------------------------------------------------------
  monitor_prologue();
  #pragma omp parallel num_threads(NUM_THREADS)
  {
    serialwork(2);
  }
  monitor_epilogue();
  CHECK(check_states(observed_states, "(ompt_state_work_parallel)+"),	\
	IMPLEMENTED_BUT_INCORRECT,					\
	"expect state parallel inside parallel regions; "		\
	"observed states '%s'",						\
	state_string(observed_states).c_str());
  
  //--------------------------------------------------------------------------
  // TEST2: In the middle of a reduction, expect to see 
  // ompt_state_work_reduction, ompt_state_work_parallel
  //--------------------------------------------------------------------------
  monitor_prologue();
  int i, accumulator = 0, N = 16;
  int master_thread_id = ompt_get_thread_id();
#pragma omp parallel for num_threads(NUM_THREADS) \
        private(i) reduction(+:accumulator)
  for(i=0; i < N; i++){
    if (ompt_get_thread_id() != master_thread_id) {
      serialwork(2);
    } else {
      serialwork(0);
    }
    accumulator+=(i*i);
  }
  monitor_epilogue(); 
  CHECK(check_states(observed_states,					\
		     "(ompt_state_work_parallel|ompt_state_work_reduction)*" \
		     "(ompt_state_wait_barrier)*"),			\
	IMPLEMENTED_BUT_INCORRECT,					\
	"expect state reduction parallel or maybe overhead in reduction " \
	"regions; observed states '%s'", \
	state_string(observed_states).c_str());
  
  //--------------------------------------------------------------------------
  // TEST3.1: Testing barrier states: ompt_state_wait_barrier_explicit
  //--------------------------------------------------------------------------
  monitor_prologue();
  #pragma omp parallel num_threads(NUM_THREADS)
  {
    if (ompt_get_thread_id() != master_thread_id) {
      serialwork(2);
    }
    
    #pragma omp barrier
    serialwork(2);
  }
  monitor_epilogue();
  
  CHECK(check_states							\
	(observed_states,						\
	 "((ompt_state_work_parallel)+"					\
	 "(ompt_state_wait_barrier|ompt_state_wait_barrier_explicit)+)"), \
	IMPLEMENTED_BUT_INCORRECT,					\
	"expect ompt_state_wait_barrier or "				\
	"ompt_state_wait_barrier_explicit here; observed '%s'",	\
	state_string(observed_states).c_str());
  
  //--------------------------------------------------------------------------
  // TEST3.2: Testing barrier states: ompt_state_wait_barrier_implicit
  //--------------------------------------------------------------------------
  monitor_prologue();
  #pragma omp parallel num_threads(NUM_THREADS)
  {
    if (ompt_get_thread_id() != master_thread_id) {
      serialwork(2);
    }
    serialwork(2);
  }
  monitor_epilogue();

  CHECK(check_states							\
	(observed_states,						\
	 "((ompt_state_work_parallel)+"					\
	 "(ompt_state_wait_barrier|ompt_state_wait_barrier_implicit)+)"), \
	IMPLEMENTED_BUT_INCORRECT,					\
	"expect ompt_state_wait_barrier or "				\
	" ompt_state_wait_barrier_implicit here; observed '%s'",	\
	state_string(observed_states).c_str());
  
  
  //--------------------------------------------------------------------------
  // TEST 4: Testing task wait states : ompt_state_wait_taskwait, 
  //         ompt_state_wait_taskgroup
  //--------------------------------------------------------------------------
#if defined(_OPENMP) && (_OPENMP >= 201307)
  monitor_prologue();
  #pragma omp parallel num_threads(NUM_THREADS)
  {
    // use taskgroup if OMP 4.0 or later
    #pragma omp taskgroup
    {
      int i, num_tasks = NUM_THREADS* 4;
      #pragma omp for
      for (i = 0; i < num_tasks; i++) {
        #pragma omp task 
	{
	  if (ompt_get_thread_id() != master_thread_id) {
	    serialwork(2);
	  } else {
	    serialwork(0);
	  }
	}
      }
      #pragma omp taskwait
      #pragma omp for
      for (i = 0; i < num_tasks; i++) {
        #pragma omp task 
	{
	  if (ompt_get_thread_id() != master_thread_id) {
	    serialwork(2);
	  } else {
	    serialwork(0);
	  }
	}
      }
    }
    #pragma omp taskwait
  }
  monitor_epilogue();

  //TODO differentiate ompt_state_wait_taskwait and ompt_state_wait_barrier ?
  CHECK(check_states(observed_states, "(ompt_state_wait_taskwait)+(.)*"	\
		     "(ompt_state_wait_taskgroup)+$"),			\
	IMPLEMENTED_BUT_INCORRECT,					\
	"expect to see ompt_state_taskwait and then "			\
	"ompt_state_taskgroup; observed '%s'",				\
	state_string(observed_states).c_str());
#endif
  
  //--------------------------------------------------------------------------
  // TEST 5.1: Testing ompt_state_wait_lock
  //--------------------------------------------------------------------------
  omp_lock_t single_lock;
  omp_init_lock(&single_lock);
  monitor_prologue();
  #pragma omp parallel num_threads(NUM_THREADS)
  {
    int i;
    #pragma omp for
    for (i = 0; i < NUM_THREADS * 4; i++) {
      #pragma omp task
      {
	omp_set_lock(&single_lock);
	usleep(rand() % 500000);
	serialwork(1);
	omp_unset_lock(&single_lock);
      }
    }
  }
  monitor_epilogue();

  CHECK(check_states(observed_states,					\
		     "(ompt_state_work_parallel)*(ompt_state_wait_lock)+" \
		     "(ompt_state_work_parallel)*"),			\
	IMPLEMENTED_BUT_INCORRECT,					\
	"expect to see a sequence of ompt_state_work_parallel and "	\
	"ompt_state_wait_lock; observed '%s'",				\
	state_string(observed_states).c_str());
  
  //--------------------------------------------------------------------------
  // TEST 5.2: Testing ompt_state_wait_nest_lock
  //--------------------------------------------------------------------------
  monitor_prologue();
  omp_nest_lock_t nest_lock;
  omp_init_nest_lock(&nest_lock);
  #pragma omp parallel num_threads(NUM_THREADS)
  {
    int i, nest_levels = 10;
    for (i = 0; i < nest_levels; i++) {
      omp_set_nest_lock(&nest_lock);
      usleep(rand() % 500000);
      serialwork(1);
      omp_unset_nest_lock(&nest_lock);
    }
  }
  omp_destroy_nest_lock(&nest_lock);
  monitor_epilogue();
  CHECK(check_states(observed_states,					\
		     "(ompt_state_work_parallel)*"			\
		     "(ompt_state_wait_nest_lock)+"			\
		     "(ompt_state_work_parallel)*"),			\
	IMPLEMENTED_BUT_INCORRECT,					\
	"expect to see a sequence of ompt_state_work_parallel and "	\
	"ompt_state_wait_nest_lock; observed '%s'",			\
	state_string(observed_states).c_str());

  //--------------------------------------------------------------------------
  // TEST 6: Testing ompt_state_wait_critical
  //--------------------------------------------------------------------------
  monitor_prologue();
  #pragma omp parallel num_threads(NUM_THREADS)
  {
    int i;
    #pragma omp for
    for (i = 0; i < NUM_THREADS * 4; i++) {
      if (ompt_get_thread_id() == master_thread_id) {
	usleep(10000);
      }
      #pragma omp critical
      serialwork(3);
    }
  }
  monitor_epilogue();

  //TODO: do we differentiate between critical and wait_lock?
  CHECK(check_states(observed_states,					\
		     "(ompt_state_work_parallel)*"			\
		     "(ompt_state_wait_critical)+"			\
		     "(ompt_state_work_parallel)*"),			\
	IMPLEMENTED_BUT_INCORRECT,					\
	"expect to see a sequence of ompt_state_work_parallel "		\
	"and wait_critical; observed '%s'",				\
	state_string(observed_states).c_str());
        
#if 0 
  // an atomic isn't necessarily observable 

  //--------------------------------------------------------------------------
  // TEST 7: Testing ompt_state_wait_atomic
  //--------------------------------------------------------------------------
  monitor_prologue();
  long atomic_number = 0;
#pragma omp parallel num_threads(NUM_THREADS)
  {
    int i;
#pragma omp for
    for (i = 0; i < NUM_THREADS * 10; i++) {
      if (ompt_get_thread_id() == master_thread_id) {
	usleep(10000);
      }
#pragma omp atomic update
      atomic_number += 1;
    }
  }
  monitor_epilogue();
  CHECK(check_states(observed_states, "(ompt_state_wait_atomic)+"), \
	IMPLEMENTED_BUT_INCORRECT,				    \
	"expect to see ompt_state_wait_atomic; observed '%s'",	    \
	state_string(observed_states).c_str());
#endif
  //--------------------------------------------------------------------------
  // TEST 8: Testing ompt_state_wait_ordered
  //--------------------------------------------------------------------------
  monitor_prologue();

  // can use a vector to check the correctness of the ordered construct
  std::vector<int> sequence; 
  #pragma omp parallel for private(i) ordered schedule(dynamic) num_threads(NUM_THREADS)
  for (i=0; i<1000; i++) 
    {
      if (ompt_get_thread_id() == master_thread_id) {
	usleep(1000);
      }
      #pragma omp ordered
      {
	sequence.push_back(i);
      }
    }
  monitor_epilogue();

  //TODO: do we differentiate between ompt_state_wait_barrier and ordered
  CHECK(check_states(observed_states, "(ompt_state_wait_ordered)+"), \
	IMPLEMENTED_BUT_INCORRECT,				     \
	"expect to see ompt_state_wait_ordered; observed '%s'",	     \
	state_string(observed_states).c_str());
     
  return 0;

}
