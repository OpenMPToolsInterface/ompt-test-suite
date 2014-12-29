#ifndef common_h
#define	common_h

#include <unistd.h>
#include <stdlib.h>

#include <omp.h>
#include <ompt.h>

#include "error.h"

/* Declaration of ompt functions */
#define macro( fn ) extern fn ## _t fn;
FOREACH_OMPT_FN( macro )
#undef macro

#if defined(__cplusplus)
extern "C" {
#endif

void warmup();
void serialwork(int workload);
int register_callback(ompt_event_t e, ompt_callback_t c);
// Callback function called after OMPT was initialized. E.g. for setting event callbacks
extern void init_test(ompt_function_lookup_t lookup);

#if defined(__cplusplus)
};
#endif

#endif	/* common_h */

