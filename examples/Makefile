PRELINK=$(prelink)

DEFAULT_LIBRARY_NAME=libomp.so
DEFAULT_COMPILER=g++

MEMOIZED_OMP_ROOT = .memoized_omp_root
MEMOIZED_OMP_LIB = .memoized_omp_lib
MEMOIZED_OMP_RPATH = .memoized_omp_rpath
MEMOIZED_OMP_COMPILER = .memoized_omp_compiler

MEMOS = \
	$(MEMOIZED_OMP_ROOT) \
	$(MEMOIZED_OMP_LIB) \
	$(MEMOIZED_OMP_RPATH) \
	$(MEMOIZED_OMP_COMPILER)

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
  rpath := $(shell cat $(wildcard $(MEMOIZED_OMP_RPATH) /dev/null))
endif

ifneq ($(lib),)
LIBRARY_NAME=$(lib)
else
LIBRARY_NAME=$(DEFAULT_LIBRARY_NAME)
endif


ifneq ($(compiler),)
CXX=$(compiler)
else
CXX=$(DEFAULT_COMPILER)
endif


# -----------------------------------------------------------------------------
#  set up the runtime
# -----------------------------------------------------------------------------
ifneq ($(openmp),)
OMP_ROOT=$(openmp)
OMP_INCLUDE = -I$(OMP_ROOT)/include

ifeq ($(rpath),yes)
OMP_LIBRARY = $(OMP_ROOT)/lib/$(LIBRARY_NAME) 
LDFLAGS = -Wl,-rpath=$(OMP_ROOT)/lib $(OMP_LIBRARY)
endif
endif

# -----------------------------------------------------------------------------
#  compiler flags
# -----------------------------------------------------------------------------
CXXFLAGS=-fopenmp -O3 -g


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
regiontest: regiontest.o Makefile
	$(PRELINK) $(CXX) $(CXXFLAGS) regiontest.o -o regiontest $(LDFLAGS)

regiontest.o:
	$(CXX) -c $(CXXFLAGS) regiontest.cpp

#
# looptest
#
looptest: looptest.cpp Makefile
	$(PRELINK) $(CXX) $(CXXFLAGS) looptest.cpp -o looptest $(LDFLAGS)


#
# parallelsections
#
parallelsections: parallelsections.cpp Makefile
	$(PRELINK) $(CXX) $(CXXFLAGS) parallelsections.cpp -o parallelsections $(LDFLAGS)


clean:
	/bin/rm -rf $(O) *.o

veryclean: clean
	/bin/rm -rf hpctoolkit-* *.hpcstruct $(MEMOS)
