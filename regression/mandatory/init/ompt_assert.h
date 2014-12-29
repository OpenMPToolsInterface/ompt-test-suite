#ifndef __ompt_assert_h__
#define __ompt_assert_h__

#ifndef OMPT_ASSERT_STREAM 
#define OMPT_ASSERT_STREAM stdout
#endif

#define OMPT_ASSERT(test_name, predicate, ...) \
  if (!(predicate)) { \
    fprintf(OMPT_ASSERT_STREAM, "%s: test failure at %s:%d\n", # test_name, __FILE__, __LINE__); \  
    fprintf(OMPT_ASSERT_STREAM, __VA_ARGS__); \
    return_code = FAILURE; \
  }

#define OMPT_SUCCESS(test_name, predicate) \
    if (predicate) fprintf(OMPT_ASSERT_STREAM, #test_name " completed successfully\n"); 


#define UNINITIALIZED -2
#define FAILURE -1
#define SUCCESS 0

int return_code = UNINITIALIZED;

#endif
