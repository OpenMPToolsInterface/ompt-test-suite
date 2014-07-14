#include <stdio.h>
#include <ompt.h>

#define OMPT_API_DECLARE(fn) fn ## _t fn

#define LOOKUP(lookup, fn) fn = (fn ## _t) lookup(#fn)

OMPT_API_DECLARE(ompt_enumerate_state);

main()
{
#pragma omp parallel
sleep(1);
}

int ompt_initialize(ompt_function_lookup_t lookup, const char *version, int ompt_version) {
  int state;
  const char *state_name;
  int ok;
  LOOKUP(lookup,ompt_enumerate_state);
  printf("enumerate ompt states:\n"
         "----------------------\n");
  for (ok = ompt_enumerate_state(ompt_state_first, &state, &state_name);
       ok; ok = ompt_enumerate_state(state, &state, &state_name)) {
    printf("state name = %s, id = %x\n", state_name, state);
  }
  return 1;
}
