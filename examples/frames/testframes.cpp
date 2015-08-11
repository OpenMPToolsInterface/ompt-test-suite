#include <assert.h>
#include <stdio.h>
#include <omp.h>
#include <ompt.h>


#define macro(x) x ## _t x;
FOREACH_OMPT_INQUIRY_FN( macro )
#undef macro

#define DEFINE_OMPT_FN_PTR( lookup, fn ) \
    fn = ( fn ## _t )lookup( #fn ); 



int
ompt_initialize( ompt_function_lookup_t lookup,
                 const char*            runtime_version,
                 unsigned int           ompt_version )
{
  // inform regression testing harness that initialization has occurred.
  ompt_initialized = 1;

  // initialize OMPT function pointers
#define macro( fn ) DEFINE_OMPT_FN_PTR( lookup, fn )
  FOREACH_OMPT_INQUIRY_FN( macro )
#undef macro

  init_test(lookup);

  quit_on_init_failure();

  return 1;
}



main()
{
	omp_set_nested(1);
        void *frame0 = __builtin_frame_address(0);
	int thread0 = omp_get_thread_num();

#pragma omp parallel 
{
	int thread1 = omp_get_thread_num();
        void *frame1 = __builtin_frame_address(0);

        void *frame1_ancestor = __builtin_frame_address(1);
	ompt_frame_t *frame1_ompt = ompt_get_task_frame(1);
	assert(frame1_ancestor == frame1_ompt->exit_runtime_frame);
	
#if 0
#pragma omp parallel 
{
        int nthreads = omp_get_num_threads();
	printf("hello world from %d (level 2) %d threads\n", rank, nthreads);
	dump_frames(rank, 2);
#pragma omp parallel 
{
        int nthreads = omp_get_num_threads();
	printf("hello world from %d (level 3) %d threads\n", rank, nthreads);
	dump_frames(rank, 3);
}
}
#endif
}
}
