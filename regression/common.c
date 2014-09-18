#include "common.h"

#define LOOKUP( lookup, fn ) fn = ( fn ## _t )lookup( #fn ); \
    if ( !fn ) \
    { \
        fprintf( stderr, "Could not lookup function %s!\n", #fn); \
	fflush(stderr); \
        assert(0); \
    }

/* Declaration of ompt functions */
#define macro( fn ) fn ## _t fn = NULL;
FOREACH_OMPT_FN( macro )
#undef macro

int
ompt_initialize( ompt_function_lookup_t lookup,
                 const char*            runtime_version,
                 int                    ompt_version )
{
    // lookup functions
    #define macro( fn ) LOOKUP( lookup, fn )
    FOREACH_OMPT_FN( macro )
    #undef macro

    init_test();

    return 1;
}
