PRELINK=$(prelink)

DEFAULT_LIBRARY_NAME=libomp.so
DEFAULT_COMPILER=g++

MEMOIZED_OMP_ROOT = .memoized_omp_root
MEMOIZED_OMP_LIB = .memoized_omp_lib
MEMOIZED_OMP_RPATH = .memoized_omp_rpath
MEMOIZED_OMP_COMPILER = .memoized_omp_compiler


# -----------------------------------------------------------------------------
#  use the memoized runtime library if one exists and none is specified
# -----------------------------------------------------------------------------
ifeq ($(openmp),)
  openmp := $(shell cat $(wildcard $(MEMOIZED_OMP_ROOT) /dev/null))
endif

ifeq ($(compiler),)
  compiler := $(shell cat $(wildcard $(MEMOIZED_OMP_COMPILER) /dev/null))
endif

ifeq ($(lib),)
  lib := $(shell cat $(wildcard $(MEMOIZED_OMP_LIB) /dev/null))
endif

ifeq ($(rpath),)
  openmp := $(shell cat $(wildcard $(MEMOIZED_OMP_RPATH) /dev/null))
endif

ifeq ($(openmp),)
 $(error Usage: make openmp=/path/to/openmp/root [lib=libname.so])
endif


ifneq ($(lib),)
LIBRARY_NAME=$(lib)
else
LIBRARY_NAME=$(DEFAULT_LIBRARY_NAME)
endif


ifneq ($(compiler),)
CXX=$(PRELINK) $(compiler)
else
CXX=$(PRELINK) $(DEFAULT_COMPILER)
endif


# -----------------------------------------------------------------------------
#  set up the runtime
# -----------------------------------------------------------------------------
OMP_ROOT=$(openmp)
OMP_INCLUDE = $(OMP_ROOT)/include

ifeq ($(rpath),yes)
OMP_LIBRARY = $(OMP_ROOT)/lib/$(LIBRARY_NAME) 
LDFLAGS = -Wl,-rpath=$(OMP_ROOT)/lib $(OMP_LIBRARY)
endif

# -----------------------------------------------------------------------------
#  compiler flags
# -----------------------------------------------------------------------------
CXXFLAGS=-fopenmp -O3 -g -I$(OMP_INCLUDE)


O=regiontest looptest parallelsections

all: memoize $(O) 

# -----------------------------------------------------------------------------
#  memoize the value for the openmp runtime path 
# -----------------------------------------------------------------------------
memoize:
	$(shell echo -n $(openmp) > $(MEMOIZED_OMP_ROOT)) 
	$(shell echo -n $(lib) > $(MEMOIZED_OMP_LIB)) 
	$(shell echo -n $(compiler) > $(MEMOIZED_OMP_COMPILER)) 
	$(shell echo -n $(rpath) > $(MEMOIZED_OMP_RPATH)) 


#
# regiontest
#
regiontest: regiontest.cpp Makefile
	$(CXX) $(CXXFLAGS) regiontest.cpp -o regiontest $(LDFLAGS)

#
# looptest
#
looptest: looptest.cpp Makefile
	$(CXX) $(CXXFLAGS) looptest.cpp -o looptest $(LDFLAGS)


#
# testframes
#
testframes: testframes.cpp Makefile
	$(CXX) $(CXXFLAGS) testframes.cpp -o testframes $(LDFLAGS)

#
# parallelsections
#
parallelsections: parallelsections.cpp Makefile
	$(CXX) $(CXXFLAGS) parallelsections.cpp -o parallelsections $(LDFLAGS)


clean:
	/bin/rm -rf $(O) *.o

veryclean: clean
	/bin/rm -rf hpctoolkit-* *.hpcstruct .memoized_openmp .memoize_openmp_lib
