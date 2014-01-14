#include <stdio.h>
#include <ompt.h>

ompt_enumerate_state_t ompt_enumerate_state;

main()
{
  int state;
  const char *state_name;
  int ok;
  printf("enumerate ompt states:\n"
         "----------------------\n");
  for (ok = ompt_enumerate_state(ompt_state_first, &state, &state_name);
       ok; ok = ompt_enumerate_state(state, &state, &state_name)) {
    printf("state name = %s, id = %x\n", state_name, state);
  }
}
