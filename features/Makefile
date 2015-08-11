DEFAULT_LIBRARY_NAME=libomp.so

MEMOIZED_OMP_ROOT = .memoized_openmp
MEMOIZED_OMP_LIB = .memoized_openmp_lib

# -----------------------------------------------------------------------------
#  use the memoized runtime library if one exists and none is specified
# -----------------------------------------------------------------------------
ifeq ($(openmp),)
  openmp := $(shell cat $(wildcard $(MEMOIZED_OMP_ROOT) /dev/null))
endif

ifeq ($(lib),)
  lib := $(shell cat $(wildcard $(MEMOIZED_OMP_LIB) /dev/null))
endif

ifeq ($(openmp),)
 $(error Usage: make openmp=/path/to/openmp/root [lib=libname.so])
endif


ifneq ($(lib),)
LIBRARY_NAME=$(lib)
else
LIBRARY_NAME=$(DEFAULT_LIBRARY_NAME)
endif


# -----------------------------------------------------------------------------
#  set up the runtime
# -----------------------------------------------------------------------------
OMP_ROOT=$(openmp)
OMP_INCLUDE = $(OMP_ROOT)/include
OMP_LIBRARY = $(OMP_ROOT)/lib/$(LIBRARY_NAME) 
LDFLAGS = -Wl,-rpath=$(OMP_ROOT)/lib $(OMP_LIBRARY)

# -----------------------------------------------------------------------------
#  compiler flags
# -----------------------------------------------------------------------------
CXXFLAGS=-O3 -g -I$(OMP_INCLUDE)



O=regiontest looptest parallelsections

all: $(O) memoize

# -----------------------------------------------------------------------------
#  memoize the value for the openmp runtime path 
# -----------------------------------------------------------------------------
memoize:
	$(shell echo -n $(openmp) > $(MEMOIZED_OMP_ROOT)) 
	$(shell echo -n $(lib) > $(MEMOIZED_OMP_LIB)) 


#
# regiontest
#
regiontest: regiontest.cpp Makefile
	g++ -fopenmp $(CXXFLAGS) regiontest.cpp -o regiontest $(LDFLAGS)

#
# looptest
#
looptest: looptest.cpp Makefile
	g++ -fopenmp $(CXXFLAGS) looptest.cpp -o looptest $(LDFLAGS)


#
# testframes
#
testframes: testframes.cpp Makefile
	g++ -fopenmp $(CXXFLAGS) testframes.cpp -o testframes $(LDFLAGS)

#
# parallelsections
#
parallelsections: parallelsections.cpp Makefile
	g++ -fopenmp $(CXXFLAGS) parallelsections.cpp -o parallelsections $(LDFLAGS)


clean:
	/bin/rm -rf $(O) *.o

veryclean: clean
	/bin/rm -rf hpctoolkit-* *.hpcstruct .memoized_openmp .memoize_openmp_lib
