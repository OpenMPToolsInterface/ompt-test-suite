# -----------------------------------------------------------------------------
#  source files for tests
# -----------------------------------------------------------------------------

TEST_SRCS	:= $(shell find . -name "test*.cpp")  

# -----------------------------------------------------------------------------
#  source and include files for utilities
# -----------------------------------------------------------------------------

UTIL_SRCS	:= $(shell find utils -name "*.cpp") 

UTIL_H		:= $(shell find utils -name "*.h") 


# -----------------------------------------------------------------------------
#  object files and executables for tests
# -----------------------------------------------------------------------------

TEST_O		= $(subst .cpp,.o,$(TEST_SRCS))
TEST_EXEC	= $(subst .cpp,,$(TEST_SRCS))


# -----------------------------------------------------------------------------
#  object files for utilities
# -----------------------------------------------------------------------------

UTIL_O		= $(subst .cpp,.o,$(UTIL_SRCS))


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

ifeq ($(compiler),icc)
  include Makefile.icc
else ifeq ($(compiler),gcc)
  include Makefile.gcc
else ifeq ($(compiler),xlc)
  include Makefile.xlc
else
  $(error Need to specify comp argument (make compiler=icc, make compiler=gcc, or make compiler=xlc))
endif


# -----------------------------------------------------------------------------
#  add include path for utility headers 
# -----------------------------------------------------------------------------
INCLUDES += -I./utils


# -----------------------------------------------------------------------------
#  build all tests
# -----------------------------------------------------------------------------

build: $(TEST_EXEC) memoize


# -----------------------------------------------------------------------------
#  run all regression tests
# -----------------------------------------------------------------------------

test: build 
	@$(foreach var,$(TEST_EXEC), \
		echo $(var); $(var) > /dev/null \
		echo $? \
	)
		
# -----------------------------------------------------------------------------
#  remove binaries
# -----------------------------------------------------------------------------

clean: memoize
	/bin/rm -f $(TEST_EXEC) $(TEST_O)
	/bin/rm -f $(UTIL_O)
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

FLAGS += -O$(LEVEL)

FLAGS += -DOMPT_DEBUG -g


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

$(TEST_EXEC):  % : %.o 
	$(CXX) $(INCLUDES) $(FLAGS) -o $@ $(OMPFLAGS) $< $(LIB) $(LIBS) $(LIBUTIL) -lrt


# -----------------------------------------------------------------------------
# rules for separate compilation of source files
# -----------------------------------------------------------------------------

%.o : %.c 
	$(CC)  -c $(INCLUDES) $(FLAGS) -o $@ $(OMPFLAGS) $<

%.o : %.cpp 
	$(CXX) -c $(INCLUDES) $(FLAGS) -o $@ $(OMPFLAGS) $<

%.o : %.f
	$(FC)  -c $(INCLUDES) $(FLAGS) -o $@ $(OMPFLAGS) $< 

