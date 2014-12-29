#include <stdio.h>
#include <omp.h>
#include <ompt.h>

#include "ompt_assert.h"

extern "C" {
#ifdef OMPT_V2013_07
int ompt_initialize(void)
{
#ifdef REPORT_CALLBACK
  printf("ompt_initialize(void) called\n");
#endif
  return_code = SUCCESS;
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
  return_code = SUCCESS;
  return 1;
}
#endif
};

int main()
{
  int max_threads = omp_get_max_threads();

  OMPT_ASSERT(test_initialize, (max_threads > 0), "ompt_get_max_threads() returned %d <= 0\n", max_threads);
  OMPT_ASSERT(test_initialize, (return_code == SUCCESS), "ompt_initialize was not called\n");

  OMPT_SUCCESS(test_initialize, (return_code == SUCCESS));

  return return_code;
}
