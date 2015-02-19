//*****************************************************************************
// system includes
//*****************************************************************************

#include <map>



//*****************************************************************************
// OpenMP runtime includes
//*****************************************************************************

#include <omp.h>



//*****************************************************************************
// regression harness includes
//*****************************************************************************

#include <ompt-regression.h>
#include <ompt-initialize.h>



//*****************************************************************************
// macros
//*****************************************************************************

#define NUM_THREADS 4


//*****************************************************************************
// global data
//*****************************************************************************

ompt_get_parallel_id_t my_ompt_get_parallel_id;



//*****************************************************************************
// interface operations
//*****************************************************************************

void 
init_test(ompt_function_lookup_t lookup) 
{
  my_ompt_get_parallel_id = 
    (ompt_get_parallel_id_t) lookup("ompt_get_parallel_id"); 
  CHECK(my_ompt_get_parallel_id, NOT_IMPLEMENTED, \
	"failed to register ompt_get_parallel_id");

  quit_on_init_failure();
}


int 
regression_test(int argc, char **argv)
{
  omp_set_nested(NESTED_VALUE);

  ompt_parallel_id_t L0 = my_ompt_get_parallel_id(0);
  
  CHECK(L0 == 0, IMPLEMENTED_BUT_INCORRECT,				\
	"before outermost parallel region, ompt_get_parallel_id "	\
	"should return 0");
  
#pragma omp parallel num_threads(NUM_THREADS)
  {
    ompt_parallel_id_t L1 = my_ompt_get_parallel_id(0);
    
    CHECK(L1 != 0, IMPLEMENTED_BUT_INCORRECT,				\
	  "in parallel region at level 1, expect a non-zero region id");
    
    CHECK(my_ompt_get_parallel_id(1) == 0, IMPLEMENTED_BUT_INCORRECT,	\
	  "in parallel region %lld at level 1, "			\
	  "expect region id 0 at depth 1; got %lld",			\
	  L1, my_ompt_get_parallel_id(1));
    
    CHECK(my_ompt_get_parallel_id(2) == 0, IMPLEMENTED_BUT_INCORRECT,	\
	  "in parallel region %lld at level 1, "			\
	  "expect an undefined region id at depth 2; got %lld",		\
	  L1, my_ompt_get_parallel_id(2));
    
    
    
#pragma omp parallel num_threads(NUM_THREADS)
    {
      ompt_parallel_id_t L2 = my_ompt_get_parallel_id(0);
      
      CHECK(L2 != 0, IMPLEMENTED_BUT_INCORRECT,				\
	    "in parallel region at level 2, expect a non-zero region id");
      
      CHECK(L1 == my_ompt_get_parallel_id(1),				\
	    IMPLEMENTED_BUT_INCORRECT,					\
	    "in parallel region %lld at level 2, "			\
	    "expect region id %lld at depth 1; got %lld",		\
	    L2, L1, my_ompt_get_parallel_id(1));
      
      CHECK(L0 == my_ompt_get_parallel_id(2),				\
	    IMPLEMENTED_BUT_INCORRECT,					\
	    "in parallel region %lld at level 2, "			\
	    "expect region id %lld at depth 2; got %lld",		\
	    L2, L0, my_ompt_get_parallel_id(2));
      
#pragma omp parallel num_threads(NUM_THREADS)
      {
	ompt_parallel_id_t L3 = my_ompt_get_parallel_id(0);
	
	CHECK(L3 != 0, IMPLEMENTED_BUT_INCORRECT,			\
	      "in parallel region at level 3, expect a non-zero region id");
	
	CHECK(L2 == my_ompt_get_parallel_id(1),		    \
	      IMPLEMENTED_BUT_INCORRECT,		    \
	      "in parallel region %lld at level 3, "	    \
	      "expect region id %lld at depth 1; got %lld", \
	      L3, L2, my_ompt_get_parallel_id(1));
	
	CHECK(L1 == my_ompt_get_parallel_id(2),				\
	      IMPLEMENTED_BUT_INCORRECT,				\
	      "in parallel region %lld at level 3, "			\
	      "expect region id %lld at depth 2; got %lld",		\
	      L3, L1, my_ompt_get_parallel_id(1));
	
	CHECK(L0 == my_ompt_get_parallel_id(3),				\
	      IMPLEMENTED_BUT_INCORRECT,				\
	      "in parallel region %lld at level 3, "			\
	      "expect region id %lld at depth 3; got %lld",		\
	      L3, L0, my_ompt_get_parallel_id(3));
	
	CHECK(L0 != L1 && L1 != L2 && L2 != L3,				\
	      IMPLEMENTED_BUT_INCORRECT,				\
	      "in parallel region %lld at level 3, "			\
	      "expect to see a different id at each level; "		\
	      "saw %lld [%lld [%lld [%lld]]]", L3, L0, L1, L2, L3);
      }
    }
  }

  L0 = my_ompt_get_parallel_id(0);
  
  CHECK(L0 == 0, IMPLEMENTED_BUT_INCORRECT,				\
	"after outermost parallel region, ompt_get_parallel_id "	\
	"should return 0; got %lld", L0);

  return return_code;
}
