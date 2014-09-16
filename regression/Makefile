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

ifeq ($(strip),yes)
STRIP=stripped
else
STRIP=unstripped
endif

ifeq ($(arch),mic)
ARCH=.knc.lin.off
OPT=-mmic
PREF=lrb
SUFFIX=mic
else
PREF=lin
SUFFIX=host
endif

#OPT += -DOMPT_DEBUG -g
OPT += -g

ifdef INTEL
ifndef INTEL_OMP_ROOT
$(error Need to define INTEL_OMP_ROOT variable)
endif

LIBDIR =$(INTEL_OMP_ROOT)/itt/libomp_oss/tmp/$(PREF)_32e-rtl_5_nor_dyn.12.$(VER).40.c0.t0$(ARCH)-$(HOST)/$(STRIP)
LIB =$(LIBDIR)/libiomp5.so -lm
LIBS=-Wl,-rpath=$(LIBDIR) $(LIB)

INCLUDES=-I$(INTEL_OMP_ROOT)/itt/libomp_oss/src

FC = ifort

else
FC = gfortran -gdwarf-3
CC = gcc -gdwarf-3
CXX = g++ -gdwarf-3

endif

HEADERS=${wildcard *.h}

EXEC = \
	test_ompt_frame \
	test_task_id-nested \
	test_task_id-simple

# EXEC = nest1-unit

EXECS=$(addsuffix .$(SUFFIX),$(EXEC))

all: $(EXECS)

clean:
	/bin/rm -f $(EXECS)

$(EXECS) : $(HEADERS) $(LIB)

fib.$(SUFFIX): fib.c 
	icc $(INCLUDES) $(OPT) -O$(LEVEL) -g -o $@ $<

luleshMPI_OMP.host: luleshMPI_OMP.cc
	mpicxx $(INCLUDES) $(OPT) -O$(LEVEL) -g -o $@ -openmp $< $(LIBS)

%.$(SUFFIX): %.c 
	icc $(INCLUDES) $(OPT) -O$(LEVEL) -g -o $@ -openmp $< $(LIBS)

%.$(SUFFIX): %.cpp 
	icpc $(INCLUDES) $(OPT) -O$(LEVEL) -g -o $@ -openmp $< $(LIBS)

%.$(SUFFIX): %.f 
	ifort $(INCLUDES) $(OPT) -O$(LEVEL) -g -o $@ -openmp $< $(LIBS)
