#include <stdio.h>
#include <omp.h>
#include <ompt.h>

#include <error.h>

int init_success = 0;

extern "C" {
#ifdef OMPT_V2013_07
int ompt_initialize(void)
{
#ifdef REPORT_CALLBACK
  printf("ompt_initialize(void) called\n");
#endif
  init_success = 1;
  return 1;
}
#else
int ompt_initialize(ompt_function_lookup_t lookup, 
                    const char *runtime_version, 
                    unsigned int ompt_version)
{
#ifdef REPORT_CALLBACK
  printf("ompt_initialize(lookup = %p, runtime_version = %s, "
         "ompt_version = %d)\n", runtime_version, ompt_version);
#endif
  init_success = 1;
  return 1;
}
#endif
};

int main(int argc, char **argv)
{
  register_segv_handler(argv);

  int max_threads = omp_get_max_threads();

  CHECK((max_threads > 0), IMPLEMENTED_BUT_INCORRECT, "ompt_get_max_threads() returned %d <= 0\n", max_threads);
  CHECK(init_success, NOT_IMPLEMENTED, "ompt_initialize was not called\n");

  return global_error_code;
}
