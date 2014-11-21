CC            = gcc
CXX           = g++
CFLAGS        = -g -O2 -Wall -Wextra -Wuninitialized -Werror=format-security \
								-D_FORTIFY_SOURCE=2
LDFLAGS       = 
CF_FLAGS			= -DUSE_TIME_MEASURE
INCLUDES      = -I./src
LIBS          = -lgmp -lgcrypt -lm -lntl
HEADERS       = $(wildcard src/*.h)
C_SOURCES     = $(wildcard src/*.c)
CPP_SOURCES   = $(wildcard src/*.cpp)
C_TESTSRCS    = $(wildcard test/*.c)
CPP_TESTSRCS  = $(wildcard test/*.cpp)
C_OBJS        = $(patsubst %.c,%.o,$(C_SOURCES))
CPP_OBJS      = $(patsubst %.cpp,%.o,$(CPP_SOURCES))
OBJS          = $(C_OBJS) $(CPP_OBJS) 

# TESTOBJS are different from OBJS because they are compiled with -DDEBUG and
# -DTIME flags and other testing flags
C_TESTOBJS    = $(patsubst %.c,%-TEST.o,$(C_TESTSRCS))
C_TESTOBJS    += $(patsubst %.c,%-TEST.o,$(filter-out src/main.c,$(C_SOURCES)))

CPP_TESTOBJS  = $(patsubst %.cpp,%-TEST.o,$(CPP_TESTSRCS))
CPP_TESTOBJS  += $(patsubst %.cpp,%-TEST.o,$(filter-out src/main.cpp,$(CPP_SOURCES)))

TESTOBJS      = $(C_TESTOBJS) $(CPP_TESTOBJS)

TEST_FLAGS    = -DDEBUG $(CF_FLAGS)

all: cf test

cf: src/cf

test: test/test

src/cf: $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

test/test: $(TESTOBJS) $(C_TESTSRCS)
	$(CXX) $(LDFLAGS) -o $@ $(TESTOBJS) $(LIBS)

# compile each C_OBJ
$(C_OBJS): $(C_SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(CF_FLAGS) $(INCLUDES) -o $@ -c $(patsubst %.o,%.c,$@)

# compile each CPP_OBJ
$(CPP_OBJS): $(CPP_SOURCES) $(HEADERS)
	$(CXX) $(CFLAGS) $(CF_FLAGS) $(INCLUDES) -o $@ -c $(patsubst %.o,%.cpp,$@)

# compile each C_TESTOBJ
$(C_TESTOBJS): $(C_SOURCES) $(C_TESTSOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(TEST_FLAGS) $(INCLUDES) -o $@ -c $(patsubst %-TEST.o,%.c,$@)

# compile each CPP_TESTOBJ
$(CPP_TESTOBJS): $(CPP_SOURCES) $(CPP_TESTSOURCES) $(HEADERS)
	$(CXX) $(CFLAGS) $(INCLUDES) $(TEST_FLAGS) -o $@ -c $(patsubst %-TEST.o,%.cpp,$@)

pdf:
	cd tex; make pdf

pdfclean:
	cd tex; make clean

clean:
	rm -f *.o
	rm -f src/*.o
	rm -f test/*.o
	rm -f test/test
	rm -f src/cf
