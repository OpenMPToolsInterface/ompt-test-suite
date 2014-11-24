EXEC = \
	testtool \
	test_task_id \
	test_event_parallel \
	test_event_thread \
	test_ompt_frame \
	test_wait_barrier

HOST:= $(shell hostname -s)

ifeq ($(opt),2)
LEVEL=2
else
LEVEL=0
endif


ifeq ($(mode),debug)
VER=deb
else
VER=rel
endif

FLAGS += -DOMPT_DEBUG -g
FLAGS += -g

ifeq ($(comp),intel)
  ifndef INTEL_OMP_ROOT
    $(error Need to define INTEL_OMP_ROOT variable)
  endif

  BDIR=$(INTEL_OMP_ROOT)/itt/libomp_oss/tmp/$(PREF)_32e-rtl_5_nor_dyn.12.$(VER).40.c0.t0$(ARCH)-$(HOST)
  LIBDIR=$(BDIR)/$(STRIP)
  LIB =$(LIBDIR)/libiomp5.so -lm
  LIBS=-Wl,-rpath=$(LIBDIR) $(LIB)

  INCLUDES=-I$(BDIR)

  FC = ifort
  CC = icc -std=c99
  CXX = icpc
  OMPFLAGS = -openmp

  ifeq ($(strip),yes)
    STRIP=stripped
  else
    STRIP=unstripped
  endif

  ifeq ($(arch),mic)
    ARCH=.knc.lin.off
    FLAGS=-mmic
    PREF=lrb
    SUFFIX=mic
  else
    PREF=lin
    SUFFIX=host
  endif

else 
 ifeq ($(comp),gcc)
   FC = gfortran -gdwarf-3
   CC = gcc -gdwarf-3
   CXX = g++ -gdwarf-3
  
   OMPFLAGS = -fopenmp

  else
     $(error Need to specify comp argument (either make comp=intel or make comp=gcc))
  endif
endif

HEADERS=${wildcard *.h}

# EXEC = nest1-unit

EXECS=$(addsuffix .$(SUFFIX),$(EXEC))

all: $(EXECS)


clean:
	/bin/rm -f $(EXECS)

$(EXECS) : $(HEADERS) $(LIB)

%.$(SUFFIX): %.c common.c
	$(CC) $(INCLUDES) $(FLAGS) -O$(LEVEL) -o $@ $(OMPFLAGS) $^ $(LIBS)

%.$(SUFFIX): %.cpp common.c
	$(CXX) $(INCLUDES) $(FLAGS) -O$(LEVEL) -o $@ $(OMPFLAGS) $^ $(LIBS)

%.$(SUFFIX): %.f
	$(FC) $(INCLUDES) $(FLAGS) -O$(LEVEL) -o $@ $(OMPFLAGS) $< $(LIBS)

