MAL_LOG_PATH = ../..

SRC_MAL_PATH = $(MAL_LOG_PATH)/src/mal_log

INCLUDE_PATH = $(MAL_LOG_PATH)/include

CXX = g++

#MAL_VERSION = 0.0.1
MAL_VERSION = `cat $(MAL_LOG_PATH)/version`

FIXED_COMPILE_FLAGS =-Wall -std=c++0x $(MAL_VARS) -I$(MAL_LOG_PATH)/src -I$(MAL_LOG_PATH)/include -fmessage-length=0 -fvisibility=hidden -fPIC -DMAL_DYNLIB_COMPILE

#override CXXFLAGS += $(FIXED_COMPILE_FLAGS)

override LDFLAGS += -Wl,--exclude-libs,ALL -shared

STAGE = stage

LIB = ./lib

INCLUDE = ./include

.PHONY: all clean debug

all : override CXXFLAGS += $(FIXED_COMPILE_FLAGS) -O3
all : BASENAME = libmal-log
all : post-build
	ln -sf $(BASENAME).so.$(MAL_VERSION) $(LIB)/$(BASENAME).so

debug : override CXXFLAGS += $(FIXED_COMPILE_FLAGS) -O0 -g3
debug : BASENAME = libmal-log-d
debug : post-build
	ln -sf $(BASENAME).so.$(MAL_VERSION) $(LIB)/$(BASENAME).so
	
pre-build : 
	mkdir -p stage
	mkdir -p $(LIB)
	ln -sf ../$(SRC_MAL_PATH)/frontend_def.hpp $(STAGE)/frontend_def.cpp;
	

post-build : so_file a_file
	ln -sf $(INCLUDE_PATH)

so_file: pre-build frontend_def.o
	$(CXX) -o $(LIB)/$(BASENAME).so.$(MAL_VERSION) $(STAGE)/frontend_def.o $(LDLIBS) $(LDFLAGS)

a_file: pre-build frontend_def.o
	$(AR) rcs $(LIB)/$(BASENAME).a.$(MAL_VERSION) $(STAGE)/frontend_def.o

frontend_def.o: $(STAGE)/frontend_def.cpp
	$(CXX) $(CXXFLAGS) -c $(STAGE)/frontend_def.cpp -o $(STAGE)/frontend_def.o

clean:
	rm -rf lib stage include


