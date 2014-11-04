// Code written by Alexandre Eichenberger @ IBM T.J. Watson for use in testing the OMPT interface

// TNUM is the thread num expected. Must set OMP_NUM_THREADS to TNUM when executing testtool
#define TNUM               4
#define HAS_START_SHUTDOWN 1  /* must have it on, since starting will init myompt */
#define DEBUG_ON           0


#define TEST_PAR_REG       1
#define TEST_MASTER	   1
#define TEST_SINGLE	   1
#define TEST_FOR_ORDERED   1
#define TEST_SECTION       1
#define TEST_CRITICAL	   1
#define TEST_ATOMIC	   1
#define TEST_LOCK	   1
#define TEST_NEST_LOCK	   1
#define TEST_CONTROL	   1
#define TEST_FLUSH	   1
#define TEST_TASK	   1
#define TEST_PRINT_STATE   1

#include <stdio.h>
#include <pthread.h>
#include "<omp.h>"
#include <assert.h>

__thread long ctid = -1;
pthread_mutex_t mutex;

#define PRINTF(_args...) {pthread_mutex_lock(&mutex);printf("<%d>", ctid);printf(_args);pthread_mutex_unlock(&mutex);}
#if DEBUG_ON
  #define DEBUG(_args...)        printf(_args)
  #define ATOMIC_DEBUG(_args...) PRINTF(_args)
#else
  #define DEBUG(_args...)
  #define ATOMIC_DEBUG(_args...) 
#endif
#define ASSERT(_x, _args...) if (!(_x)) { \
  pthread_mutex_lock(&mutex); \
  printf("\n\n<%d>ERROR: ", ctid); printf(_args); printf("\n\n");	\
  pthread_mutex_unlock(&mutex); \
  assert(_x); }
#define TRUE  1
#define FALSE 0
#define MIN(_x, _y) ((_x)<(_y) ? (_x) : (_y))
#define MAX(_x, _y) ((_x)>(_y) ? (_x) : (_y))

typedef enum ompt_set_callback_rc_e { /* non standard */
  ompt_set_callback_error = 0,
  ompt_has_event_no_callback = 1, 
  ompt_no_event_no_callback = 2,
  ompt_has_event_may_callback = 3,
  ompt_has_event_must_callback = 4
} ompt_set_callback_rc_t;

typedef ompt_state_t (*ompt_get_state_t)(ompt_wait_id_t *wait_id);
typedef ompt_thread_id_t (*ompt_get_thread_id_t)();
typedef void *(*ompt_get_idle_frame_t)();
typedef ompt_parallel_id_t (*ompt_get_parallel_id_t)(int ancestor_level);
typedef int (*ompt_get_parallel_team_size_t)(int ancestor_level);
typedef ompt_task_id_t (*ompt_get_task_id_t)(int ancestor_level);
typedef ompt_frame_t *(*ompt_get_task_frame_t)(int ancestor_level);
typedef int (*ompt_enumerate_state_t)(int current_state, 
  int *next_state, const char **next_state_name);
typedef int (*ompt_set_callback_t)(ompt_event_t event, ompt_callback_t callback);
typedef int (*ompt_get_callback_t)(ompt_event_t event, ompt_callback_t *callback);

////////////////////////////////////////////////////////////////////////////////
// inquiries functions

ompt_get_state_t myompt_get_state = NULL;
ompt_get_thread_id_t myompt_get_thread_id = NULL;
ompt_get_idle_frame_t myompt_get_idle_frame = NULL;
ompt_get_parallel_id_t myompt_get_parallel_id = NULL;
ompt_get_parallel_team_size_t myompt_get_parallel_team_size = NULL;
ompt_get_task_id_t myompt_get_task_id = NULL;
ompt_get_task_frame_t myompt_get_task_frame = NULL;
ompt_set_callback_t myompt_set_callback = NULL;
ompt_get_callback_t myompt_get_callback_t = NULL;
ompt_enumerate_state_t myompt_enumerate_state = NULL;

void SetInquiryFunctions(ompt_function_lookup_t lookup)
{
  myompt_get_state = (ompt_get_state_t)lookup("ompt_get_state"); 
  assert(myompt_get_state != NULL);
  myompt_get_thread_id = (ompt_get_thread_id_t)lookup("ompt_get_thread_id");
  assert(myompt_get_thread_id != NULL);
  myompt_get_idle_frame = (ompt_get_idle_frame_t)lookup("ompt_get_idle_frame"); 
  assert(myompt_get_idle_frame != NULL);
  myompt_get_parallel_id = (ompt_get_parallel_id_t)lookup("ompt_get_parallel_id"); 
  assert(myompt_get_parallel_id != NULL);
  myompt_get_parallel_team_size = (ompt_get_parallel_team_size_t)lookup("ompt_get_parallel_team_size"); 
  assert(myompt_get_parallel_team_size != NULL);
  myompt_get_task_id = (ompt_get_task_id_t)lookup("ompt_get_task_id"); 
  assert(myompt_get_task_id != NULL);
  myompt_get_task_frame = (ompt_get_task_frame_t)lookup("ompt_get_task_frame"); 
  assert(myompt_get_task_frame != NULL);
  myompt_set_callback = (ompt_set_callback_t)lookup("ompt_set_callback"); 
  assert(myompt_set_callback != NULL);
  myompt_get_callback_t = (ompt_get_callback_t)lookup("ompt_get_callback"); 
  assert(myompt_get_callback_t != NULL);
  myompt_enumerate_state = (ompt_enumerate_state_t)lookup("ompt_enumerate_state");
  assert(myompt_enumerate_state != NULL);
}

////////////////////////////////////////////////////////////////////////////////
// data and its init

typedef struct toolData_s {
  // init once
  long threadId;
  long threadBeginCount;
  long threadEndCount;
  void *threadIdleFrame;
  // init per experiment
  uint64_t parCtid;
  uint64_t parId;
  uint64_t parentTaskIdforParallel;
  uint64_t taskId;
  long parBeginCount;
  long parEndCount;
  int parentTeamSize;
  int childParentTeamSize;
  int childTeamSize;
  int requestedTeamSize;
  void *parentRentry;
  void *childEnd;
  void *fct;
  long implicitTaskBeginCount;
  long implicitTaskEndCount;
  long explicitTaskBeginCount;
  long explicitTaskEndCount;
  long barrierBeginCount;
  long barrierEndCount;
  long waitBarrierBeginCount;
  long waitBarrierEndCount;
  // lock
  ompt_wait_id_t wait_id;
  long acquireLockCount;
  long releaseLockCount;
  long initLockCount;
  long destroyLockCount;
  // nest lock
  ompt_wait_id_t nest_wait_id;
  long acquireNestLockCount;
  long releaseNestLockCount;
  long acquireInnerNestLockCount;
  long releaseInnerNestLockCount;
  long initNestLockCount;
  long destroyNestLockCount;
  // workshare & master count
  long singleInCount;
  long singleOutCount;
  long sectionBeginCount;
  long sectionEndCount;
  long loopBeginCount;
  long loopEndCount;
  long masterBeginCount;
  long masterEndCount;
  // control
  uint64_t first;
  uint64_t second;
  // idle
  long idleBeginCount;
  long idleEndCount;
  // critical
  ompt_wait_id_t criticalWaitId;
  long criticalCount;
  // ordered
  ompt_wait_id_t orderedWaitId;
  long orderedCount;
  // atomic
  ompt_wait_id_t atomicWaitId;
  long atomicCount;
  // tasks
  ompt_task_id_t parentTaskId;
  ompt_task_id_t childTaskId;
  void *taskFct;
  // flush
  long flushCount;
} toolData_t;

void InitOneData(toolData_t *data, long first)
{
  ATOMIC_DEBUG("init data");
  if (first) {
    data->threadId = 0;
    data->threadBeginCount = data->threadEndCount = 0;
    data->threadIdleFrame = 0;
    data->idleBeginCount = 0;
    data->idleEndCount = 0;
  }
  data->parCtid = -1;
  data->parId =0;
  data->parentTaskIdforParallel = 0;
  data->taskId =0;
  data->parBeginCount =0 ;
  data->parEndCount = 0;
  data->parentTeamSize = 0;
  data->childParentTeamSize = 0;
  data->childTeamSize = 0;
  data->requestedTeamSize = 0;
  data->parentRentry = NULL;
  data->childEnd = NULL;
  data->fct = NULL;
  data->implicitTaskBeginCount =0;
  data->implicitTaskEndCount =0;
  data->explicitTaskBeginCount =0;
  data->explicitTaskEndCount =0;
  data->barrierBeginCount =0;
  data->barrierEndCount =0;
  data->waitBarrierBeginCount =0;
  data->waitBarrierEndCount =0;
  // lock
  data->wait_id = 0;
  data->acquireLockCount= 0;
  data->releaseLockCount= 0;
  data->initLockCount= 0;
  data->destroyLockCount= 0;
  // nest lock
  data->nest_wait_id = 0;
  data->acquireNestLockCount = 0;
  data->releaseNestLockCount = 0;
  data->acquireInnerNestLockCount = 0;
  data->releaseInnerNestLockCount = 0;
  data->initNestLockCount = 0;
  data->destroyNestLockCount = 0;
  // workshares
  data->singleInCount = 0;
  data->singleOutCount = 0;
  data->sectionBeginCount = 0;
  data->sectionEndCount = 0;
  data->loopBeginCount = 0;
  data->loopEndCount = 0;
  data->masterBeginCount = 0;
  data->masterEndCount = 0;
  data->first = 0;
  data->second = 0;
  data->criticalWaitId = 0;
  data->criticalCount  = 0;
  data->orderedWaitId = 0;
  data->orderedCount  = 0;
  data->atomicWaitId = 0;
  data->atomicCount  = 0;
  // task
  data->parentTaskId = 0;
  data->childTaskId = 0;
  data->taskFct = NULL;
  // flush
  data->flushCount = 0;
}

// global data
toolData_t toolData[TNUM];
long ctidNum;
long initTool = 0;
long masterEndSupported = 0;

void InitAllData(int first) 
{
  ATOMIC_DEBUG("Init start\n");
  int t;
  if (first) {
    pthread_mutex_init(&mutex, NULL);
    ctidNum = 0;
  } else {
    long myVal = toolData[ctid].implicitTaskEndCount;
    for(t=0; t<TNUM; t++) {
      volatile long *otherVal = &toolData[t].implicitTaskEndCount;
      if (*otherVal != myVal) {
	DEBUG("Wait for ctid %d to reach implicitTaskEndCount of %ld\n", t, myVal);
        while (*otherVal != myVal);
      }
    }
  }
  for(int t=0; t<TNUM; t++) InitOneData(&toolData[t], first);
  ATOMIC_DEBUG("Init done\n");
}

////////////////////////////////////////////////////////////////////////////////
// atomic ops (non omp to avoid interference with test)

long FetchAndInc(long *addr, char *str) 
{
  long oldVal;
  pthread_mutex_lock(&mutex);
  oldVal = *addr;
  *addr += 1;
  //printf("  value for %s: %lld -> %lld\n", str, oldVal, oldVal+1);
  pthread_mutex_unlock(&mutex);
  return oldVal;
}

////////////////////////////////////////////////////////////////////////////////
// state check

void CheckState(ompt_state_t expectedState, ompt_wait_id_t expectedWait, long hasWait, char *str)
{
  ompt_wait_id_t currWait;
  ompt_state_t currState = myompt_get_state(&currWait);

  ASSERT(expectedState == currState, 
    "##check state error (%s): expected %d, got %d (state)\n", str,
    (int) expectedState, (int) currState);
  if (hasWait) {
    ASSERT(expectedWait == currWait, 
      "##check state error (%s): expected %d, got %d (wait id assoc with state %d)\n",
      str, (int) expectedWait, (int) currWait, (int) expectedState);
  } 
}

////////////////////////////////////////////////////////////////////////////////
// frame check

void PrintFrames()
{
  void *fp = __builtin_frame_address(0);
  if (fp) { printf("  depth 0: 0x%llx\n",  (uint64_t) fp); fp = __builtin_frame_address(1); }
  if (fp) { printf("  depth 1: 0x%llx\n",  (uint64_t) fp); fp = __builtin_frame_address(2); }
  if (fp) { printf("  depth 2: 0x%llx\n",  (uint64_t) fp); fp = __builtin_frame_address(3); }
  if (fp) { printf("  depth 3: 0x%llx\n",  (uint64_t) fp); fp = __builtin_frame_address(4); }
  if (fp) { printf("  depth 4: 0x%llx\n",  (uint64_t) fp); fp = __builtin_frame_address(5); }
  if (fp) { printf("  depth 5: 0x%llx\n",  (uint64_t) fp); fp = __builtin_frame_address(6); }
  if (fp) { printf("  depth 6: 0x%llx\n",  (uint64_t) fp); fp = __builtin_frame_address(7); }
  if (fp) { printf("  depth 7: 0x%llx\n",  (uint64_t) fp); fp = __builtin_frame_address(8); }
  if (fp) { printf("  depth 8: 0x%llx\n",  (uint64_t) fp); fp = __builtin_frame_address(9); }
  if (fp) { printf("  depth 9: 0x%llx\n",  (uint64_t) fp); fp = __builtin_frame_address(10);}
  if (fp) { printf("  depth 10: 0x%llx\n", (uint64_t) fp); fp = __builtin_frame_address(11);}
  if (fp) { printf("  depth 11: 0x%llx\n", (uint64_t) fp); fp = __builtin_frame_address(12);}
  if (fp) { printf("  depth 12: 0x%llx\n", (uint64_t) fp); fp = __builtin_frame_address(13);}
  if (fp) { printf("  depth 13: 0x%llx\n", (uint64_t) fp); fp = __builtin_frame_address(14);}
  if (fp) { printf("  depth 14: 0x%llx\n", (uint64_t) fp); fp = __builtin_frame_address(15);}
  if (fp) { printf("  depth 15: 0x%llx\n", (uint64_t) fp); fp = __builtin_frame_address(16);}
}

long CheckFrameBeforeXisY(void *x, void *y, char *str)
{
  ASSERT(x, "expected valid x frame");
  ASSERT(y, "expected valid y frame");
  #if 0
    pthread_mutex_lock(&mutex);
    printf("CheckFrame: (%s) verify if frame before 0x%llx is 0x%llx\n", 
      str, (uint64_t) x,  (uint64_t) y);
    PrintFrames();
    pthread_mutex_unlock(&mutex);
  #endif

  long depth = -1;
  if (__builtin_frame_address(0) == x) {
    ASSERT(__builtin_frame_address(1) == y, "expected frame 0x%llx, got 0x%llx (1)", 
      y, __builtin_frame_address(1));
    depth = 0;
  } else if (__builtin_frame_address(1) == x) {
    ASSERT(__builtin_frame_address(2) == y, "expected frame 0x%llx, got 0x%llx (2)", 
      y, __builtin_frame_address(2));
    depth = 1;
  } else if (__builtin_frame_address(2) == x) {
    ASSERT(__builtin_frame_address(3) == y, "expected frame 0x%llx, got 0x%llx (3)", 
        y, __builtin_frame_address(3));
    depth = 2;
  } else if (__builtin_frame_address(3) == x) {
    ASSERT(__builtin_frame_address(4) == y, "expected frame 0x%llx, got 0x%llx (4)", 
        y, __builtin_frame_address(5));
    depth = 3;
  } else if (__builtin_frame_address(4) == x) {
    ASSERT(__builtin_frame_address(5) == y, "expected frame 0x%llx, got 0x%llx (5)", 
        y, __builtin_frame_address(5));
    depth = 4;
  } else if (__builtin_frame_address(5) == x) {
    ASSERT(__builtin_frame_address(6) == y, "expected frame 0x%llx, got 0x%llx (6)", 
        y, __builtin_frame_address(6));
    depth = 5;
  } else if (__builtin_frame_address(6) == x) {
    ASSERT(__builtin_frame_address(7) == y, "expected frame 0x%llx, got 0x%llx (7)", 
        y, __builtin_frame_address(7));
    depth = 6;
  } else if (__builtin_frame_address(7) == x) {
    ASSERT(__builtin_frame_address(8) == y, "expected frame 0x%llx, got 0x%llx (8)", 
        y, __builtin_frame_address(8));
    depth = 7;
  } else if (__builtin_frame_address(8) == x) {
    ASSERT(__builtin_frame_address(9) == y, "expected frame 0x%llx, got 0x%llx (9)", 
        y, __builtin_frame_address(9));
    depth = 8;
  } else if (__builtin_frame_address(9) == x) {
    ASSERT(__builtin_frame_address(10) == y, "expected frame 0x%llx, got 0x%llx (10)", 
        y, __builtin_frame_address(10));
    depth = 9;
  } else {
    ASSERT(0, "too deep for test");
  }
  return depth;
}

////////////////////////////////////////////////////////////////////////////////
// print supported states

void PrintSupportedStates()
{
  printf("\nList supported states:\n");
  int state;
  const char *state_name;
  for(int ok = myompt_enumerate_state(ompt_state_first, &state, &state_name); 
      ok; ok = myompt_enumerate_state(state, &state, &state_name)) {
    printf("  %4x/%4d: %s\n", state, state, state_name);
  }
  printf("done\n");
}


////////////////////////////////////////////////////////////////////////////////
// support

void QuickCheckPar()
{
  int tnum = omp_get_num_threads();
  ASSERT(tnum == TNUM, "wanted %d threads, got %d\n", TNUM, tnum);
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  ASSERT(ctid>=0, "out of bound ctid %ld, min 0", ctid);
}

void Wait()
{
  PRINTF("    start sleeping\n");
  usleep(10*1000); 
  PRINTF("    completed sleeping\n");
}


////////////////////////////////////////////////////////////////////////////////
// mandatory callbacks + ones needed anyway

void CallbackShutdown()
{
  for(int t=1; t<TNUM; t++) {
    ASSERT(toolData[t].threadEndCount == 1, "missing thread end");
  }
  // reset ctid num to indicate shutdown
  ctidNum = -1;
}

void CallbackThreadBegin(ompt_thread_type_t type, ompt_thread_id_t thread_id)
{
  ctid = FetchAndInc(&ctidNum, "get ctid number");
  PRINTF("    Creating a thread with id %ld (id from RT %lld)\n", ctid, thread_id);
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  toolData[ctid].threadBeginCount = 1;
  toolData[ctid].threadIdleFrame = myompt_get_idle_frame();
  ASSERT(toolData[ctid].threadIdleFrame, "expected a defined thread idle frame");
  ASSERT(thread_id, "expected defined thread id");
  ASSERT(myompt_get_thread_id() == thread_id, "mismatch thread id");
  toolData[ctid].threadId = thread_id;
}

void CallbackThreadEnd(ompt_thread_type_t type, ompt_thread_id_t thread_id)
{
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  ASSERT(toolData[ctid].threadBeginCount == 1, "missing thread begin");
  toolData[ctid].threadEndCount = 1;
  ASSERT(thread_id, "expected defined thread id");
  ASSERT(myompt_get_thread_id() == thread_id, "mismatch thread id");
}

// parallel region
void CallbackParallelBegin ( 
  ompt_task_id_t parent_task_id,    
  ompt_frame_t *parent_task_frame,  
  ompt_parallel_id_t parallel_id,   
  uint32_t requested_team_size,
  void *parallel_function)          
{
  ATOMIC_DEBUG("CallbackParallelBegin\n");
  ASSERT(parent_task_id, "undefined task id %lld", parent_task_id);
  ASSERT(parallel_id, "undef parallel id %lld", parallel_id);
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  toolData[ctid].parId = parallel_id;
  toolData[ctid].parentTaskIdforParallel = parent_task_id;
  toolData[ctid].parBeginCount++;
  toolData[ctid].parentTeamSize = myompt_get_parallel_team_size(0);
  toolData[ctid].parentRentry = parent_task_frame->reenter_runtime_frame;
  toolData[ctid].requestedTeamSize = requested_team_size;
  ASSERT(toolData[ctid].requestedTeamSize == TNUM, "bad team size");
  ASSERT(parallel_function, "expected a parallel function\n");
  toolData[ctid].fct = parallel_function;
  ASSERT(myompt_get_task_id(0) == parent_task_id, 
    "bad parent task id query %lld != %lld", myompt_get_task_id(0), parent_task_id);
  ASSERT(myompt_get_parallel_id(0) != parallel_id, "bad parallel id, new par id must be different");
  ASSERT(myompt_get_task_frame(0) == parent_task_frame, "bad parent task frame query");
  toolData[ctid].childTaskId = parent_task_id;
  // store ctid of this thread
  toolData[ctid].parCtid = ctid;
}								    

void CallbackParallelEnd ( 
  ompt_parallel_id_t parallel_id,   
  ompt_task_id_t parent_task_id)          
{
  ATOMIC_DEBUG("CallbackParallelEnd\n");
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  ASSERT(parent_task_id, "undefined task id %lld", parent_task_id);
  ASSERT(toolData[ctid].parentTaskIdforParallel == parent_task_id, "parent id not same as in begin");
  ASSERT(toolData[ctid].parId == parallel_id, "diff par id");
  toolData[ctid].parEndCount++;
  ASSERT(myompt_get_task_id(0) == parent_task_id, "bad parent task id query");
  ASSERT(toolData[ctid].parCtid == ctid, "same thread should start/end par region");
}								    

void CallbackImplicitBegin(
  ompt_parallel_id_t parallel_id, 
  ompt_task_id_t  task_id)
{
  ATOMIC_DEBUG("CallbackImplicitBegin, setting par id to %lld\n", parallel_id);
  ASSERT(parallel_id, "undefined parallel id %lld", parallel_id);
  ASSERT(task_id, "undefined task id %lld", task_id);
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  toolData[ctid].childParentTeamSize = myompt_get_parallel_team_size(1);
  toolData[ctid].childTeamSize = myompt_get_parallel_team_size(0);
  toolData[ctid].implicitTaskBeginCount++;
  toolData[ctid].parId = parallel_id;
  toolData[ctid].taskId = task_id;
  ASSERT(myompt_get_parallel_id(0) == parallel_id, "bad par id query");
  ASSERT(myompt_get_task_id(0) == task_id, "bad child task id query");
  // support for explicit tasks
  toolData[ctid].parentTaskId = task_id;
  toolData[ctid].childTaskId = 0;
}

void CallbackImplicitEnd(
  ompt_parallel_id_t parallel_id, 
  ompt_task_id_t  task_id)
{
  ATOMIC_DEBUG("CallbackImplicitEnd\n");
  ASSERT(parallel_id, "undefined parallel id %lld", parallel_id);
  ASSERT(task_id, "undefined task id %lld", task_id);
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  ASSERT(toolData[ctid].parId == parallel_id, 
    "diff par id in implicit, wanted %lld, got  %lld", 
    toolData[ctid].parId, parallel_id);
  ASSERT(toolData[ctid].taskId == task_id, "diff task id");
  ASSERT(myompt_get_parallel_id(0) == parallel_id, "bad par id query");
  ASSERT(myompt_get_task_id(0) == task_id, "bad child task id query");
  // support for explicit tasks
  toolData[ctid].parentTaskId = 0;
  toolData[ctid].childTaskId = 0;
  // msync
  __lwsync();
  toolData[ctid].implicitTaskEndCount++;
}

// barrier
void CallbackBarrierBegin(
  ompt_parallel_id_t parallel_id, 
  ompt_task_id_t  task_id)
{
  ATOMIC_DEBUG("CallbackBarrierBegin\n");
  ASSERT(parallel_id, "undefined parallel id %lld", parallel_id);
  ASSERT(task_id, "undefined task id %lld", task_id);
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  ASSERT(toolData[ctid].parId == parallel_id, 
    "diff par id in implicit, wanted %lld, got  %lld", 
    toolData[ctid].parId, parallel_id);
  ASSERT(toolData[ctid].taskId == task_id, "diff task id");
  toolData[ctid].barrierBeginCount++;
}

void CallbackBarrierEnd(
  ompt_parallel_id_t parallel_id, 
  ompt_task_id_t  task_id)
{
  ATOMIC_DEBUG("CallbackBarrierEnd\n");
  ASSERT(parallel_id, "undefined parallel id %lld", parallel_id);
  ASSERT(task_id, "undefined task id %lld", task_id);
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  ASSERT(toolData[ctid].parId == parallel_id, 
    "diff par id in implicit, wanted %lld, got  %lld", 
    toolData[ctid].parId, parallel_id);
  ASSERT(toolData[ctid].taskId == task_id, "diff task id");
  toolData[ctid].barrierEndCount++;
  ASSERT(toolData[ctid].barrierBeginCount == toolData[ctid].barrierEndCount, 
    "bad barrier begin/end count");;
}

void CallbackWaitBarrierBegin(
  ompt_parallel_id_t parallel_id, 
  ompt_task_id_t  task_id)
{
  ATOMIC_DEBUG("CallbackWaitBarrierBegin\n");
  ASSERT(parallel_id, "undefined parallel id %lld", parallel_id);
  ASSERT(task_id, "undefined task id %lld", task_id);
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  ASSERT(toolData[ctid].parId == parallel_id, 
    "diff par id in implicit, wanted %lld, got  %lld", 
    toolData[ctid].parId, parallel_id);
  ASSERT(toolData[ctid].taskId == task_id, "diff task id");
  toolData[ctid].waitBarrierBeginCount++;
  CheckState(ompt_state_wait_barrier_implicit, 0, FALSE, "barrier wait begin");
}

void CallbackWaitBarrierEnd(
  ompt_parallel_id_t parallel_id, 
  ompt_task_id_t  task_id)
{
  ATOMIC_DEBUG("CallbackWaitBarrierEnd\n");
  ASSERT(parallel_id, "undefined parallel id %lld", parallel_id);
  ASSERT(task_id, "undefined task id %lld", task_id);
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  ASSERT(toolData[ctid].parId == parallel_id, 
    "diff par id in implicit, wanted %lld, got  %lld", 
    toolData[ctid].parId, parallel_id);
  ASSERT(toolData[ctid].taskId == task_id, "diff task id");
  toolData[ctid].waitBarrierEndCount++;
  // don't have to be in wait when ending???
  // start, definitely yes; end, we are already in the next state
  //CheckState(ompt_state_wait_barrier, 0, FALSE, "barrier wait end");
}

// idle
void CallbackIdleBegin(uint64_t threadId)
{
  ATOMIC_DEBUG("CallbackIdleBegin\n");
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  CheckState(ompt_state_idle, 0, FALSE, "idle");
  toolData[ctid].idleBeginCount++;
}

void CallbackIdleEnd(uint64_t threadId)
{
  ATOMIC_DEBUG("CallbackIdleEnd\n");
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  toolData[ctid].idleEndCount++;
  ASSERT(toolData[ctid].idleBeginCount == toolData[ctid].idleEndCount, 
    "idle mismatch, got %ld & %lldfor thread %ld", toolData[ctid].idleBeginCount, 
    toolData[ctid].idleEndCount, ctid);
}

////////////////////////////////////////////////////////////////////////////////
// test for parallel region

#if TEST_PAR_REG

void TestParallelRegion()
{
  printf("\n>>> Test Parallel Region: start\n");
  InitAllData(FALSE);
  void *currFrame = __builtin_frame_address(0);

  ompt_state_t parState = (TNUM==1) ? ompt_state_work_serial : ompt_state_work_parallel;
  CheckState(ompt_state_work_serial, 0, FALSE, "serial");

  #pragma omp parallel num_threads(TNUM)
  {
    QuickCheckPar();
    // verify thread init
    ASSERT(toolData[ctid].threadBeginCount == 1, "missing thread begin");
    if (omp_get_thread_num()==0) {
      CheckFrameBeforeXisY(toolData[ctid].parentRentry, currFrame, "master check");
      ASSERT(toolData[ctid].parBeginCount == 1, "missing par begin");
      ASSERT(toolData[ctid].parentTeamSize == 1, "bad parent team size %d\n",
        toolData[ctid].parentTeamSize);
    }
    ASSERT(toolData[ctid].implicitTaskBeginCount == 1, "missing implicit begin for t %d", ctid);
    ASSERT(toolData[ctid].childParentTeamSize == 1, "bad parent team size %d\n",
       toolData[ctid].childParentTeamSize);
    ASSERT(toolData[ctid].childTeamSize == TNUM, "bad team size %d\n",
       toolData[ctid].childTeamSize);
    // verify frame info
    void *outlinedFrame =  __builtin_frame_address(0);
    ompt_frame_t *innerFrame = myompt_get_task_frame(0);
    ASSERT(innerFrame->reenter_runtime_frame==NULL, "expected inner reenter null");
    ompt_frame_t *outerFrame  = myompt_get_task_frame(1);
    ASSERT(outerFrame->reenter_runtime_frame == toolData[0].parentRentry, "bad reentry");
    long depth = CheckFrameBeforeXisY(outlinedFrame, innerFrame->exit_runtime_frame, 
      "implicit thread");
    //ASSERT(depth==0, "expected depth of 0, got %ld", depth);
    ASSERT(myompt_get_parallel_id(0), "expected a defined parallel id");
    ASSERT(!myompt_get_parallel_id(1), "expected an unefined parent parallel id");
    ASSERT(myompt_get_thread_id() == toolData[ctid].threadId, "mismatch thread id");
  }
  // hoping to sleep long enough so that all threads are finished
  Wait();

  ASSERT(ctidNum=TNUM, "failed to begin %ld threads, got %ld\n", TNUM, ctidNum);
  ASSERT(toolData[ctid].parEndCount == 1, "missing par end");
  ompt_parallel_id_t parallel_id = toolData[0].parId;
  for(int t=0; t<TNUM; t++) {
    // parallel check
    ASSERT(toolData[t].parId == parallel_id, "bad parallel id, got %lld, expected %lld for iter %d", 
      (uint64_t) toolData[t].parId, (uint64_t) parallel_id, t);
    ASSERT(toolData[t].implicitTaskEndCount == 1, "missing implicit end");
    // barrier
    ASSERT(toolData[t].barrierBeginCount == toolData[t].barrierEndCount, "bad barrier beging/end count");
    // no task: 0 or 1
    ASSERT(toolData[t].waitBarrierBeginCount == toolData[t].waitBarrierEndCount,
      "non matching wait barrier begin/end count");;
  }
  CheckState(ompt_state_work_serial, 0, FALSE, "serial");
  ASSERT(!myompt_get_parallel_id(0), "expected a defined parallel id");
  printf("  Test Parallel Region: success\n");
}

#endif

////////////////////////////////////////////////////////////////////////////////
// Master

void CallbackMasterBegin(
  ompt_parallel_id_t parallel_id, 
  ompt_task_id_t  task_id)
{
  ATOMIC_DEBUG("CallbackMasterBegin\n");
  ASSERT(parallel_id, "undefined parallel id %lld", parallel_id);
  ASSERT(task_id, "undefined task id %lld", task_id);
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  ASSERT(toolData[ctid].parId == parallel_id, 
    "diff par id in implicit, wanted %lld, got  %lld", 
    toolData[ctid].parId, parallel_id);
  ASSERT(toolData[ctid].taskId == task_id, "diff task id");
  toolData[ctid].masterBeginCount++;
}

void CallbackMasterEnd(
  ompt_parallel_id_t parallel_id, 
  ompt_task_id_t  task_id)
{
  ATOMIC_DEBUG("CallbackMasterEnd\n");
  ASSERT(parallel_id, "undefined parallel id %lld", parallel_id);
  ASSERT(task_id, "undefined task id %lld", task_id);
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  ASSERT(toolData[ctid].parId == parallel_id, 
    "diff par id in implicit, wanted %lld, got  %lld", 
    toolData[ctid].parId, parallel_id);
  ASSERT(toolData[ctid].taskId == task_id, "diff task id");
  toolData[ctid].masterEndCount++;
  ASSERT(toolData[ctid].masterBeginCount == toolData[ctid].masterEndCount, "master mismatch");
}


#if TEST_MASTER
void TestMaster()
{
  printf("\n>>> Test Master: start\n");
  InitAllData(FALSE);

  #pragma omp parallel num_threads(TNUM)
  {
    QuickCheckPar();
    #pragma omp master
    {
      ASSERT(toolData[ctid].masterBeginCount == 1, "master begin expected");
    }
  }
  // test
  if (masterEndSupported) {
    ASSERT(toolData[ctid].masterEndCount == 1, "master end expected");
  }
  printf("  Test Master: success\n");
}
#endif

////////////////////////////////////////////////////////////////////////////////
// Single

void CallbackSingleIn(
  ompt_parallel_id_t parallel_id, 
  ompt_task_id_t  task_id)
{
  ATOMIC_DEBUG("CallbackSingleIn\n");
  ASSERT(parallel_id, "undefined parallel id %lld", parallel_id);
  ASSERT(task_id, "undefined task id %lld", task_id);
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  ASSERT(toolData[ctid].parId == parallel_id, "diff par id in implicit");
  ASSERT(toolData[ctid].taskId == task_id, "diff task id");
  toolData[ctid].singleInCount++;
}

void CallbackSingleOut(
  ompt_parallel_id_t parallel_id, 
  ompt_task_id_t  task_id)
{
  ATOMIC_DEBUG("CallbackSingleOut\n");
  ASSERT(parallel_id, "undefined parallel id %lld", parallel_id);
  ASSERT(task_id, "undefined task id %lld", task_id);
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  ASSERT(toolData[ctid].parId == parallel_id, "diff par id in implicit");
  ASSERT(toolData[ctid].taskId == task_id, "diff task id");
  toolData[ctid].singleOutCount++;
}

#if TEST_SINGLE
void TestSingle()
{
  printf("\n>>> Test Single: start\n");
  InitAllData(FALSE);

  #pragma omp parallel num_threads(TNUM)
  {
    QuickCheckPar();
    #pragma omp single
    {
      ASSERT(toolData[ctid].singleInCount == 1, "single begin expected");
    }
  }
  // test
  long expectedIn = 0;
  long expectedOut = 0;
  for(int t=0; t<TNUM; t++) {
    // single
    expectedIn += toolData[t].singleInCount;
    expectedOut += toolData[t].singleOutCount;
  }
  ASSERT(expectedIn == 2, "expected one x2 single, got %d\n", expectedIn);
  ASSERT(expectedOut == 2*(TNUM-1), "expected %d x2 single out, got %d\n", TNUM-1, expectedOut);
  printf("  Test Single: success\n");
}
#endif

////////////////////////////////////////////////////////////////////////////////
// Loop Ordered

void CallbackLoopBegin(
  ompt_parallel_id_t parallel_id, 
  ompt_task_id_t  task_id)
{
  ATOMIC_DEBUG("CallbackLoopBegin\n");
  ASSERT(parallel_id, "undefined parallel id %lld", parallel_id);
  ASSERT(task_id, "undefined task id %lld", task_id);
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  ASSERT(toolData[ctid].parId == parallel_id, "diff par id in implicit");
  ASSERT(toolData[ctid].taskId == task_id, "diff task id");
  toolData[ctid].loopBeginCount++;
}

void CallbackLoopEnd(
  ompt_parallel_id_t parallel_id, 
  ompt_task_id_t  task_id)
{
  ATOMIC_DEBUG("CallbackLoopEnd\n");
  ASSERT(parallel_id, "undefined parallel id %lld", parallel_id);
  ASSERT(task_id, "undefined task id %lld", task_id);
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  ASSERT(toolData[ctid].parId == parallel_id, "diff par id in implicit");
  ASSERT(toolData[ctid].taskId == task_id, "diff task id");
  toolData[ctid].loopEndCount++;
  ASSERT(toolData[ctid].loopBeginCount == toolData[ctid].loopEndCount, "loop mismatch");
}

void CallbackOrderedWait(ompt_wait_id_t wait_id)
{
  ATOMIC_DEBUG("CallbackOrderedWait\n");
  CheckState(ompt_state_wait_ordered, wait_id, TRUE, "ordered wait");
}

void CallbackOrderedAcquired(ompt_wait_id_t wait_id)
{
  ATOMIC_DEBUG("CallbackOrderedAcquired\n");
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  toolData[ctid].orderedWaitId = wait_id;
  toolData[ctid].orderedCount++;
  ASSERT(toolData[ctid].orderedCount % 2 == 1, "expected odd ordered count id");
}

void CallbackOrderedRelease(ompt_wait_id_t wait_id)
{
  ATOMIC_DEBUG("CallbackOrderedRelease\n");
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  ASSERT(toolData[ctid].orderedWaitId == wait_id, 
    "ordered wait id mismatch, wanted %lld, got %lld", 
    toolData[ctid].orderedWaitId, wait_id);
  toolData[ctid].orderedCount++;
  ASSERT(toolData[ctid].orderedCount % 2 == 0, "expected even ordered count id");
}

#if TEST_FOR_ORDERED
void TestForOrdered()
{
  int loopTripCount = MAX(1, TNUM-1);
  printf("\n>>> Test For Ordered (%ld iter): start\n", loopTripCount);
  InitAllData(FALSE);

  #pragma omp parallel num_threads(TNUM)
  {
    QuickCheckPar();
    #pragma omp for schedule(static, 1) ordered
    for(int i=0; i<loopTripCount; i++) {
      // one will have none
      #pragma omp ordered
      {
        ASSERT(toolData[ctid].loopBeginCount == 1, "loop begin expected");
      }
    }

  }
  // test
  long loopOrderedTot = 0;
  ompt_wait_id_t orderedWaitId = toolData[0].orderedWaitId;
  for(int t=0; t<TNUM; t++) {
    ASSERT(toolData[t].loopBeginCount == 1, "bad loop begin");
    ASSERT(toolData[t].loopEndCount == 1, "bad loop end");
    // ordered
    if (toolData[t].orderedCount) {
      // may not have had an iter
      ASSERT(toolData[t].orderedWaitId == orderedWaitId, 
        "bad ordered id, got %lld for thread %d", toolData[t].orderedWaitId, t);
    }
    loopOrderedTot += toolData[t].orderedCount;
  }
  if (TNUM > 1) {
    // check really the number of times lock was
    // aquired/released... here once per chunk assigned to a thread.
     ASSERT( loopOrderedTot == 2 * loopTripCount, 
      "bad ordered total, expected 2x trip count, got %ld\n", loopOrderedTot);
  }
  printf("  Test Ordered: success\n");
}
#endif


////////////////////////////////////////////////////////////////////////////////
// Section

void CallbackSectionBegin(
  ompt_parallel_id_t parallel_id, 
  ompt_task_id_t  task_id)
{
  ATOMIC_DEBUG("CallbackSectionBegin\n");
  ASSERT(parallel_id, "undefined parallel id %lld", parallel_id);
  ASSERT(task_id, "undefined task id %lld", task_id);
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  ASSERT(toolData[ctid].parId == parallel_id, "diff par id in implicit");
  ASSERT(toolData[ctid].taskId == task_id, "diff task id");
  toolData[ctid].sectionBeginCount++;
}

void CallbackSectionEnd(
  ompt_parallel_id_t parallel_id, 
  ompt_task_id_t  task_id)
{
  ATOMIC_DEBUG("CallbackSectionEnd\n");
  ASSERT(parallel_id, "undefined parallel id %lld", parallel_id);
  ASSERT(parallel_id, "undefined parallel id %lld", parallel_id);
  ASSERT(task_id, "undefined task id %lld", task_id);
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  ASSERT(toolData[ctid].parId == parallel_id, "diff par id in implicit");
  ASSERT(toolData[ctid].taskId == task_id, "diff task id");
  toolData[ctid].sectionEndCount++;
  ASSERT(toolData[ctid].sectionBeginCount == toolData[ctid].sectionEndCount, "section mismatch");
}

#if TEST_SECTION
void TestSection()
{
  printf("\n>>> Test Section: start\n");
  InitAllData(FALSE);

  #pragma omp parallel num_threads(TNUM)
  {
    QuickCheckPar();
    #pragma omp sections
    {
      #pragma omp section
      {
        ASSERT(toolData[ctid].sectionBeginCount == 1, "single begin expected");
      }
      #pragma omp section
      {
        ASSERT(toolData[ctid].sectionBeginCount == 1, "single begin expected");
      }
      #pragma omp section
      {
        ASSERT(toolData[ctid].sectionBeginCount == 1, "single begin expected");
      }
    }
  }
  // test
  long loopOrderedTot = 0;
  ompt_wait_id_t orderedWaitId = toolData[0].orderedWaitId;
  for(int t=0; t<TNUM; t++) {
    ASSERT(toolData[t].sectionBeginCount == 1, "bad loop begin");
    ASSERT(toolData[t].sectionEndCount == 1, "bad loop end");
  }
  printf("  Test Secton: success\n");
}
#endif


////////////////////////////////////////////////////////////////////////////////
// Critical

void CallbackCriticalWait(ompt_wait_id_t wait_id)
{
  CheckState(ompt_state_wait_critical, wait_id, TRUE, "critical wait");
}

void CallbackCriticalAcquired(ompt_wait_id_t wait_id)
{
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  toolData[ctid].criticalWaitId = wait_id;
  toolData[ctid].criticalCount++;
  ASSERT(toolData[ctid].criticalCount % 2 == 1, "expected odd critical count id");
}

void CallbackCriticalRelease(ompt_wait_id_t wait_id)
{
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  ASSERT(toolData[ctid].criticalWaitId == wait_id, "critical wait id mismatch");
  toolData[ctid].criticalCount++;
  ASSERT(toolData[ctid].criticalCount % 2 == 0, "expected even critical count id");
}

#if TEST_CRITICAL
void TestCritical()
{
  printf("\n>>> Test Critical: start\n");
  InitAllData(FALSE);

  #pragma omp parallel num_threads(TNUM)
  {
    QuickCheckPar();
    #pragma omp critical
    {
      ASSERT(toolData[ctid].criticalCount % 2 == 1, 
        "expected odd critical count id, got %ld, by thread %d",
        toolData[ctid].criticalCount, ctid);
    }
  }
  // test
  ompt_wait_id_t criticalWaitId = toolData[0].criticalWaitId;
  for(int t=0; t<TNUM; t++) {
    ASSERT(toolData[t].criticalWaitId == criticalWaitId, "bad critical id");
    ASSERT(toolData[t].criticalCount == 2, "expected critical count = 2");
  }
  printf("  Test Critical: success\n");
}
#endif

////////////////////////////////////////////////////////////////////////////////
// Atomic

void CallbackAtomicWait(ompt_wait_id_t wait_id)
{
  CheckState(ompt_state_wait_atomic, wait_id, TRUE, "atomic wait");
}

void CallbackAtomicAcquired(ompt_wait_id_t wait_id)
{
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  toolData[ctid].atomicWaitId = wait_id;
  toolData[ctid].atomicCount++;
  ASSERT(toolData[ctid].atomicCount % 2 == 1, "expected odd atomic count id");
}

void CallbackAtomicRelease(ompt_wait_id_t wait_id)
{
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  ASSERT(toolData[ctid].atomicWaitId == wait_id, "atomic wait id mismatch");
  toolData[ctid].atomicCount++;
  ASSERT(toolData[ctid].atomicCount % 2 == 0, "expected even atomic count id");
}

#if TEST_ATOMIC
void TestAtomic()
{
  printf("\n>>> Test Atomic: start\n");
  InitAllData(FALSE);

  long atomicData = 0;
  #pragma omp parallel num_threads(TNUM)
  {
    QuickCheckPar();
    #pragma omp atomic
    atomicData++;

  }
  // test
  for(int t=0; t<TNUM; t++) {
    ASSERT(toolData[t].atomicCount == 2, "expected atomic count = 2");
    ASSERT(toolData[t].atomicWaitId == (ompt_wait_id_t) &atomicData, "bad atomic address");
  }
  printf("  Test Atomic: success\n");
}
#endif


////////////////////////////////////////////////////////////////////////////////
// locks

// lock
void CallbackInitLock(ompt_wait_id_t wait_id)
{
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  toolData[ctid].initLockCount++;
}

void CallbackWaitLock(ompt_wait_id_t wait_id)
{
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  ASSERT(toolData[ctid].wait_id == wait_id, "bad wait id");
  CheckState(ompt_state_wait_lock, 0, FALSE, "wait lock");
}

void CallbackAcquireLock(ompt_wait_id_t wait_id)
{
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  ASSERT(toolData[ctid].wait_id == wait_id, "bad wait id");
  toolData[ctid].acquireLockCount++;
}

void CallbackReleaseLock(ompt_wait_id_t wait_id)
{
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  ASSERT(toolData[ctid].wait_id == wait_id, "bad wait id");
  toolData[ctid].releaseLockCount++;
}

void CallbackDestroyLock(ompt_wait_id_t wait_id)
{
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  ASSERT(toolData[ctid].wait_id == wait_id, "bad wait id");
  toolData[ctid].destroyLockCount++;
}

omp_lock_t myLock;

#if TEST_LOCK
void TestLock()
{
  printf("\n>>> Test Locks: start\n");
  InitAllData(FALSE);

  omp_init_lock(&myLock);
  ompt_state_t parState = (TNUM==1) ? ompt_state_work_serial : ompt_state_work_parallel;
  #pragma omp parallel num_threads(TNUM)
  {
    QuickCheckPar();
    // expected wait id for locks
    toolData[ctid].wait_id = (ompt_wait_id_t) &myLock;
    // lock
    omp_set_lock(&myLock);
    // something to do
    CheckState(parState, 0, FALSE, "work parallel");
    // unlock
    omp_unset_lock(&myLock);
  }
  omp_destroy_lock(&myLock);
  // test
  for(int t=0; t<TNUM; t++) {
    int masterOnly = (t==0) ? 1 : 0;
    // lock
    ASSERT(toolData[t].acquireLockCount == toolData[t].releaseLockCount,
      "non matching acquire/release lock  count");
    ASSERT(toolData[t].acquireLockCount == 1, "acquire lock count != 1");
    ASSERT(toolData[t].initLockCount == masterOnly, 
      "bad lock init at t = %d, got %ld", t, toolData[t].initLockCount);
    ASSERT(toolData[t].destroyLockCount == masterOnly, 
      "bad lock destroy at t = %d, got %ld", t, toolData[t].destroyLockCount);
  }
  printf("  Test Lock: success\n");
}
#endif

////////////////////////////////////////////////////////////////////////////////
// nest locks

// nest lock
void CallbackInitNestLock(ompt_wait_id_t wait_id)
{
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  toolData[ctid].initNestLockCount++;
  ASSERT(wait_id, "bad wait id");
}

void CallbackWaitNestLock(ompt_wait_id_t wait_id)
{
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  ASSERT(wait_id, "bad wait id");
  CheckState(ompt_state_wait_nest_lock, 0, FALSE, "wait lock");
}

// nest lock outer
void CallbackAcquireNestLockFirst(ompt_wait_id_t wait_id)
{
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  ASSERT(wait_id, "bad wait id");
  // set id
  toolData[ctid].nest_wait_id = wait_id;
  toolData[ctid].acquireNestLockCount++;
}

void CallbackReleaseNestLockLast(ompt_wait_id_t wait_id)
{
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  ASSERT(toolData[ctid].nest_wait_id == wait_id, "bad wait id");
  toolData[ctid].releaseNestLockCount++;
}

// nest lock inner
void CallbackAcquireNestLockNext(ompt_wait_id_t wait_id)
{
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  ASSERT(toolData[ctid].nest_wait_id == wait_id, "bad wait id");
  toolData[ctid].acquireInnerNestLockCount++;
}

void CallbackReleaseNestLockPrev(ompt_wait_id_t wait_id)
{
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  ASSERT(toolData[ctid].nest_wait_id == wait_id, "bad wait id");
  toolData[ctid].releaseInnerNestLockCount++;
}

void CallbackDestroyNestLock(ompt_wait_id_t wait_id)
{
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  ASSERT(toolData[ctid].nest_wait_id == wait_id, 
    "bad wait id, got 0x%llx, wanted 0x%llx", 
    (uint64_t)wait_id, (uint64_t) toolData[ctid].nest_wait_id);
  toolData[ctid].destroyNestLockCount++;
}

omp_nest_lock_t myNestLock;

#if TEST_NEST_LOCK
void TestNestLock()
{
  printf("\n>>> Test Nest Locks: start\n");
  InitAllData(FALSE);

  omp_init_nest_lock(&myNestLock);
  ompt_state_t parState = (TNUM==1) ? ompt_state_work_serial : ompt_state_work_parallel;
  #pragma omp parallel num_threads(TNUM)
  {
    QuickCheckPar();
    // nest locks
    omp_set_nest_lock(&myNestLock);
    omp_set_nest_lock(&myNestLock);
    // something to do
    CheckState(parState, 0, FALSE, "work parallel");
    omp_unset_nest_lock(&myNestLock);
    omp_unset_nest_lock(&myNestLock);
  }
  omp_destroy_nest_lock(&myNestLock);
  // test
  for(int t=0; t<TNUM; t++) {
    int masterOnly = (t==0) ? 1 : 0;
    // nest lock
    ASSERT(toolData[t].acquireNestLockCount == toolData[t].releaseNestLockCount,
      "non matching acquire/release nest lock count");
    ASSERT(toolData[t].acquireNestLockCount == 1, "acquire nest lock count != 1");
    ASSERT(toolData[t].acquireInnerNestLockCount == toolData[t].releaseInnerNestLockCount,
      "non matching acquire/release nest lock count");
    ASSERT(toolData[t].acquireInnerNestLockCount == 1, "acquire inner nest lock count != 1");
    ASSERT(toolData[t].initNestLockCount == masterOnly, 
      "bad lock init at t = %d, got %ld", t, toolData[t].initNestLockCount);
    ASSERT(toolData[t].destroyNestLockCount == masterOnly, 
      "bad lock destroy at t = %d, got %ld", t, toolData[t].destroyNestLockCount);
  }
  printf("  Test Nest Lock: success\n");
}
#endif

////////////////////////////////////////////////////////////////////////////////
// control

void CallbackControl(uint64_t first, uint64_t second)
{
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  toolData[ctid].first = first;
  toolData[ctid].second = second;
}

#if TEST_CONTROL
void TestControl()
{
  printf("\n>>> Test Control: start\n");
  InitAllData(FALSE);

  ompt_control(1, 2);
  ASSERT(toolData[ctid].first == 1, "bad first control");
  ASSERT(toolData[ctid].second == 2, "bad first control");
  printf("  Test Control: success\n");
}
#endif


////////////////////////////////////////////////////////////////////////////////
// flush

void CallbackFlush(ompt_thread_id_t thread_id)
{
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  toolData[ctid].flushCount++;
  ASSERT(thread_id, "expected defined thread id");
  ASSERT(myompt_get_thread_id() == thread_id, "mismatch thread id");
}

#if TEST_FLUSH
void TestFlush()
{
  printf("\n>>> Test Flush: start\n");
  InitAllData(FALSE);

  #pragma omp flush
  ASSERT(toolData[ctid].flushCount == 1, "bad flush count");
  printf("  Test Flush: success\n");
}
#endif

////////////////////////////////////////////////////////////////////////////////
// test for task region

void CallbackTaskBegin(
  ompt_task_id_t parent_task_id,    /* id of parent task            */
  ompt_frame_t *parent_task_frame,  /* frame data for parent task   */
  ompt_task_id_t new_task_id,       /* id of begind task           */
  void *new_task_function           /* pointer to outlined function */
  )
{
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  ASSERT(parent_task_id, "undef parent task");
  ASSERT(toolData[ctid].parentTaskId == parent_task_id, "bad parent");
  ASSERT(new_task_id, "undef parent task");
  // from now on, this task is a potential parent
  toolData[ctid].parentTaskId = new_task_id;
}

void CallbackTaskEnd(ompt_task_id_t  task_id)
{
  ASSERT(ctid<TNUM, "out of bound ctid %ld, max %ld", ctid, TNUM);
  ASSERT(task_id, "expected a task to end");
  toolData[ctid].parentTaskId = 0;
}

void CallbackTaskSwitch(
  ompt_task_id_t suspended_task_id, /* id of suspended task         */ 
  ompt_task_id_t resumed_task_id    /* id of resumed task           */
  )
{
  ASSERT(resumed_task_id, "expected a task to resume");
  // from now on, this new task is a potential parent
  toolData[ctid].parentTaskId = resumed_task_id;
}

#if TEST_TASK
void TestTaskRegion()
{
  printf("\n>>> Test Task Region: start\n");
  InitAllData(FALSE);

  void *currFrame = __builtin_frame_address(0);

  ompt_state_t parState = (TNUM==1) ? ompt_state_work_serial : ompt_state_work_parallel;
  CheckState(ompt_state_work_serial, 0, FALSE, "serial");

  #pragma omp parallel num_threads(TNUM)
  {
    int tnum = omp_get_num_threads();
    ASSERT(tnum == TNUM, "wanted %d threads, got %d\n", TNUM, tnum);

    if (omp_get_thread_num()==0) {
      CheckFrameBeforeXisY(toolData[ctid].parentRentry, currFrame, "master check");
      ASSERT(toolData[ctid].parBeginCount == 1, "missing par begin");

      // master only
      #pragma omp task
      {
	CheckState(parState, 0, FALSE, "parallel in task 1");
        #pragma omp task
        {
  	  CheckState(parState, 0, FALSE, "parallel in task 2");
        }
      }
    } // master
  } // parallel
  printf("  Test Task Region: done\n");
}
#endif

////////////////////////////////////////////////////////////////////////////////
// register callbacks

void RegisterCallback(ompt_event_t e, ompt_callback_t c)
{
  printf("use set_callback(event, addr) 0x%llx(%d, 0x%llx)\n",
    (uint64_t) myompt_set_callback, e, (uint64_t)c);
  int rc = myompt_set_callback(e, c);
  if (e == ompt_event_master_end) {
    masterEndSupported = (rc == ompt_has_event_must_callback);
  } else {
    ASSERT(rc>1, "expected support for callback");
  }
}

extern int ompt_initialize(
  ompt_function_lookup_t lookup, 
  const char *runtime_version_string, 
  unsigned int ompt_version)
{
  printf("OMPT IS INITIALIZING: lookup functions with runtime version %s and ompt version %d\n", 
    runtime_version_string, ompt_version);
  SetInquiryFunctions(lookup);

  initTool = TRUE;
  // init tool data
  InitAllData(TRUE);
  // register callbacks
  printf("OMPT IS INITIALIZING: register callback\n");
  RegisterCallback(ompt_event_runtime_shutdown      , (ompt_callback_t) CallbackShutdown); 
  RegisterCallback(ompt_event_thread_begin         , (ompt_callback_t) CallbackThreadBegin); 
  RegisterCallback(ompt_event_thread_end           , (ompt_callback_t) CallbackThreadEnd); 
  RegisterCallback(ompt_event_parallel_begin       , (ompt_callback_t) CallbackParallelBegin); 
  RegisterCallback(ompt_event_parallel_end         , (ompt_callback_t) CallbackParallelEnd); 
  RegisterCallback(ompt_event_implicit_task_begin  , (ompt_callback_t) CallbackImplicitBegin); 
  RegisterCallback(ompt_event_implicit_task_end    , (ompt_callback_t) CallbackImplicitEnd); 
  // barrier
  RegisterCallback(ompt_event_barrier_begin         , (ompt_callback_t) CallbackBarrierBegin); 
  RegisterCallback(ompt_event_barrier_end           , (ompt_callback_t) CallbackBarrierEnd); 
  RegisterCallback(ompt_event_wait_barrier_begin    , (ompt_callback_t) CallbackWaitBarrierBegin); 
  RegisterCallback(ompt_event_wait_barrier_end      , (ompt_callback_t) CallbackWaitBarrierEnd);
  // locks 
  RegisterCallback(ompt_event_init_lock             , (ompt_callback_t) CallbackInitLock); 
  RegisterCallback(ompt_event_wait_lock             , (ompt_callback_t) CallbackWaitLock); 
  RegisterCallback(ompt_event_acquired_lock         , (ompt_callback_t) CallbackAcquireLock); 
  RegisterCallback(ompt_event_release_lock          , (ompt_callback_t) CallbackReleaseLock); 
  RegisterCallback(ompt_event_destroy_lock          , (ompt_callback_t) CallbackDestroyLock); 
  // nest_locks 
  RegisterCallback(ompt_event_init_nest_lock        , (ompt_callback_t) CallbackInitNestLock); 
  RegisterCallback(ompt_event_wait_nest_lock        , (ompt_callback_t) CallbackWaitNestLock); 
  RegisterCallback(ompt_event_acquired_nest_lock_first,(ompt_callback_t) CallbackAcquireNestLockFirst); 
  RegisterCallback(ompt_event_release_nest_lock_last,(ompt_callback_t) CallbackReleaseNestLockLast); 
  RegisterCallback(ompt_event_acquired_nest_lock_next, (ompt_callback_t) CallbackAcquireNestLockNext); 
  RegisterCallback(ompt_event_release_nest_lock_prev, (ompt_callback_t) CallbackReleaseNestLockPrev); 
  RegisterCallback(ompt_event_destroy_nest_lock     , (ompt_callback_t) CallbackDestroyNestLock); 
  // single
  RegisterCallback(ompt_event_single_in_block_begin , (ompt_callback_t) CallbackSingleIn);
  RegisterCallback(ompt_event_single_in_block_end   , (ompt_callback_t) CallbackSingleIn);
  RegisterCallback(ompt_event_single_others_begin   , (ompt_callback_t) CallbackSingleOut);
  RegisterCallback(ompt_event_single_others_end     , (ompt_callback_t) CallbackSingleOut);
  RegisterCallback(ompt_event_master_begin          , (ompt_callback_t) CallbackMasterBegin); 
  RegisterCallback(ompt_event_master_end            , (ompt_callback_t) CallbackMasterEnd); 
  RegisterCallback(ompt_event_loop_begin            , (ompt_callback_t) CallbackLoopBegin); 
  RegisterCallback(ompt_event_loop_end              , (ompt_callback_t) CallbackLoopEnd); 
  RegisterCallback(ompt_event_sections_begin        , (ompt_callback_t) CallbackSectionBegin); 
  RegisterCallback(ompt_event_sections_end          , (ompt_callback_t) CallbackSectionEnd); 
  // control
  RegisterCallback(ompt_event_control               , (ompt_callback_t) CallbackControl); 
  // flush
  RegisterCallback(ompt_event_flush                 , (ompt_callback_t) CallbackFlush); 
  // idle
  RegisterCallback(ompt_event_idle_begin            , (ompt_callback_t) CallbackIdleBegin); 
  RegisterCallback(ompt_event_idle_end              , (ompt_callback_t) CallbackIdleEnd); 
  // critical
  RegisterCallback(ompt_event_wait_critical         , (ompt_callback_t) CallbackCriticalWait); 
  RegisterCallback(ompt_event_acquired_critical     , (ompt_callback_t) CallbackCriticalAcquired); 
  RegisterCallback(ompt_event_release_critical      , (ompt_callback_t) CallbackCriticalRelease); 
  // ordered
  RegisterCallback(ompt_event_wait_ordered          , (ompt_callback_t) CallbackOrderedWait); 
  RegisterCallback(ompt_event_acquired_ordered      , (ompt_callback_t) CallbackOrderedAcquired); 
  RegisterCallback(ompt_event_release_ordered       , (ompt_callback_t) CallbackOrderedRelease); 
  // atomic
  RegisterCallback(ompt_event_wait_atomic           , (ompt_callback_t) CallbackAtomicWait); 
  RegisterCallback(ompt_event_acquired_atomic       , (ompt_callback_t) CallbackAtomicAcquired); 
  RegisterCallback(ompt_event_release_atomic        , (ompt_callback_t) CallbackAtomicRelease); 
  // tasks
  RegisterCallback(ompt_event_task_begin           , (ompt_callback_t) CallbackTaskBegin); 
  RegisterCallback(ompt_event_task_end             , (ompt_callback_t) CallbackTaskEnd); 
  RegisterCallback(ompt_event_task_switch           , (ompt_callback_t) CallbackTaskSwitch); 
  printf("OMPT IS INITIALIZING: done\n");

  return 1;
}


////////////////////////////////////////////////////////////////////////////////
// main

int main ()
{ 
  int t;
  ompt_wait_id_t currWait;

  #if HAS_START_SHUTDOWN
    // start runtime
    _lomp_Runtime_Start();
    ASSERT(initTool, "expected tool to be init, it is not; check lib and OMP_TOOL");
  #endif

  #if TEST_PRINT_STATE
    PrintSupportedStates();
  #endif
  
  #if TEST_PAR_REG 
    TestParallelRegion();
  #endif
  
  #if TEST_MASTER
    TestMaster();
  #endif
  
  #if TEST_SINGLE
    TestSingle();
  #endif
  
  #if TEST_FOR_ORDERED
    TestForOrdered();
  #endif
  
  #if TEST_SECTION
    TestSection();
  #endif

  #if TEST_CRITICAL
    TestCritical();
  #endif
  
  #if TEST_ATOMIC
    TestAtomic();
  #endif
  
  #if TEST_LOCK
    TestLock();
  #endif
  
  #if TEST_NEST_LOCK
    TestNestLock();
  #endif
  
  #if TEST_CONTROL
    TestControl();
  #endif

  #if TEST_FLUSH
    TestFlush();
  #endif
  
  #if TEST_TASK
    TestTaskRegion();
  #endif

  // shutdown
  #if HAS_START_SHUTDOWN
    // stop runtiem
    _lomp_Runtime_Stop();
    ASSERT(ctidNum == -1, "failed to shut down runtime\n");
    // check idle
    for(int t=0; t<TNUM; t++) {
      if (t!=0) {
	// init thread does not idle
        ASSERT(toolData[t].idleEndCount>0, "assumed that we woudl have idle at least once (thread %d)", t);
      }
    }
  #endif

  printf("successful test, return 55\n\n\n");
  return 55;
}
