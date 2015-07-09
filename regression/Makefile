# -----------------------------------------------------------------------------
#  source files for tests
# -----------------------------------------------------------------------------

MI_SRCS	:= \
	$(shell find mandatory/init -name "test*.cpp") 

ME_SRCS	:= \
	$(shell find mandatory/events -name "test*.cpp") \
	$(shell find mandatory/events -name "test*.c") 

MQ_SRCS	:= \
	$(shell find mandatory/inquiry_functions -name "test*.cpp") 

O_SRCS := \
	$(shell find optional -name "test*.cpp") 

TEST_SRCS	:= \
	$(MI_SRCS) \
	$(ME_SRCS) \
	$(MQ_SRCS) \
	$(O_SRCS)

ALL_EXEC  := \
	$(shell find mandatory -executable -type f -name "test*") \
	$(shell find optional  -executable -type f -name "test*") 

ALL_DOT_O  := \
	$(shell find mandatory -type f -name "*.o") \
	$(shell find optional  -type f -name "*.o") \
	$(shell find utils     -type f -name "*.o") 


# -----------------------------------------------------------------------------
#  source and include files for utilities
# -----------------------------------------------------------------------------

UTIL_SRCS	:= \
	$(shell find utils -name "*.cpp") \
	$(shell find utils -name "*.c") \
	$(shell find utils -name "*.f")

UTIL_H		:= $(shell find utils -name "*.h") 


# -----------------------------------------------------------------------------
#  object files and executables for tests
# -----------------------------------------------------------------------------

TEST_EXEC	:= $(addsuffix .exe,$(basename $(TEST_SRCS)))
TEST_O		= $(addsuffix .o,$(TEST_EXEC))


# -----------------------------------------------------------------------------
#  object files for utilities
# -----------------------------------------------------------------------------

UTIL_O		= $(addsuffix .o,$(basename $(UTIL_SRCS)))


# -----------------------------------------------------------------------------
#  support library containing utiliies
# -----------------------------------------------------------------------------

LIBUTIL		= utils/libutil.a


# -----------------------------------------------------------------------------
#  include Makefile with rules for each compiler
# -----------------------------------------------------------------------------

MEMOIZED_COMPILER = .memoized_compiler

ifndef ($(compiler),)
  compiler := $(shell cat $(wildcard $(MEMOIZED_COMPILER) /dev/null))
endif

ifeq ($(compiler),)
  compiler := $(CC)
endif
ifeq ($(compiler),icc)
  ifeq ($(arch),mic)
    ARCH_FLAGS=-mmic
  endif
  include Makefile.icc
else ifeq ($(compiler),gcc)
  include Makefile.gcc
else ifeq ($(compiler),clang)
  include Makefile.clang
else ifeq ($(compiler),xlc)
  include Makefile.xlc
else
  $(error Need to specify compiler argument: make compiler=icc, make compiler=gcc, make compiler=clang or make compiler=xlc)
endif


# -----------------------------------------------------------------------------
#  add include path for utility headers 
# -----------------------------------------------------------------------------
INCLUDES += -I./utils
INCLUDES += -I../ompt


# -----------------------------------------------------------------------------
#  build all tests
# -----------------------------------------------------------------------------

build: $(TEST_EXEC) memoize


# -----------------------------------------------------------------------------
#  run all regression tests
# -----------------------------------------------------------------------------

test: build runonly

runonly:
	@python utils/run_all_tests.py


# -----------------------------------------------------------------------------
#  remove binaries
# -----------------------------------------------------------------------------

clean: memoize
	/bin/rm -f $(ALL_DOT_O) $(ALL_EXEC)
	/bin/rm -f $(LIBUTIL)



# -----------------------------------------------------------------------------
#  memoize the value for a compiler so you don't have to supply it every time
# -----------------------------------------------------------------------------
memoize:
	$(shell echo -n $(compiler) > $(MEMOIZED_COMPILER)) 


# -----------------------------------------------------------------------------
#  compute compiler flags for utilities and tests 
# -----------------------------------------------------------------------------

ifeq ($(opt),2)
  LEVEL=2
else
  LEVEL=0
endif

BASE_FLAGS += $(ARCH_FLAGS)
BASE_FLAGS += -O$(LEVEL)
BASE_FLAGS += -DOMPT_DEBUG -g
BASE_FLAGS += $(INCLUDES) 

FLAGS = $(OMPFLAGS) $(BASE_FLAGS)


# -----------------------------------------------------------------------------
#  rules for building library of utilities
# -----------------------------------------------------------------------------

$(UTIL_O): $(UTIL_H)

$(LIBUTIL): $(UTIL_O)
	ar r $@ $(UTIL_O) 


# -----------------------------------------------------------------------------
#  rules for building tests
# -----------------------------------------------------------------------------

$(TEST_EXEC): $(UTIL_H) $(UTIL_SRCS) $(LIBUTIL)

$(TEST_EXEC):  %.exe : %.o 
	$(CXX) $(FLAGS) -o $@ $< $(LIBUTIL) $(LIB) $(LIBS) -lrt


# -----------------------------------------------------------------------------
# rules for separate compilation of source files
# -----------------------------------------------------------------------------

%ompt-regression.o : %ompt-regression.c 
	$(CC)  -c $(BASE_FLAGS) -o $@ $<

%.o : %.c 
	$(CC)  -c $(FLAGS) -o $@ $<

%.o : %.cpp 
	$(CXX) -c $(FLAGS) -o $@ $<

%.o : %.f
	$(FC)  -c $(FLAGS) -o $@ $< 

