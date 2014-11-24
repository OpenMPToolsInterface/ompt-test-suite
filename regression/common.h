#ifndef COMMON_H
#define	COMMON_H

#include <ompt.h>
#include <stdio.h>
#include <assert.h>

// Macro to set an OMPT callback (input event name e.g. "loop_begin", will set "on_loop_begin" as callback function)
#define REG_CB( EVENT ) \
    if ( ompt_set_callback( ompt_event_##EVENT, ( ompt_callback_t )on_##EVENT ) != ompt_set_result_event_may_occur_callback_always ) \
    { \
        fprintf(stderr, "Failed to register OMPT callback %s!\n", #EVENT ); \
	fflush(stderr); \
        assert(false); \
    }

/* Declaration of ompt functions */
#define macro( fn ) extern fn ## _t fn;
FOREACH_OMPT_FN( macro )
#undef macro

// Callback function after OMPT was initialized. E.g. for setting event callbacks
extern int init_test(ompt_function_lookup_t lookup,
                     const char*            runtime_version,
                     unsigned int           ompt_version );


#endif	/* COMMON_H */

