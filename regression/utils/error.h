#ifndef check_h
#define	check_h

#include <pthread.h>
#include <stdio.h>

#define TRUE  1
#define FALSE 0

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define CHECK(__condition, __error_code, ...) if (!(__condition)) { \
    pthread_mutex_lock(&assert_mutex); \
    printf("  %s: error at %s:%d",  executable_name, __FILE__, __LINE__); printf(" -- "  __VA_ARGS__ ); printf("\n"); \
    pthread_mutex_unlock(&assert_mutex); \
    global_error_code = MIN(global_error_code, __error_code); }

#define NOT_IMPLEMENTED -2
#define IMPLEMENTED_BUT_INCORRECT -1 
#define CORRECT 0

#if defined(__cplusplus)
extern "C" {
#endif

extern pthread_mutex_t thread_mutex;
extern pthread_mutex_t assert_mutex;
extern int global_error_code;

extern const char *executable_name;
void register_segv_handler(char **argv);

#if defined(__cplusplus)
};
#endif


#endif	/* check_h */

