UFO_LOG_PATH = ../..

SRC_UFO_PATH = $(UFO_LOG_PATH)/src/ufo_log

INCLUDE_PATH = $(UFO_LOG_PATH)/include

CXX = g++

UFO_VERSION = 0.0.1

FIXED_COMPILE_FLAGS =-Wall -std=c++0x $(UFO_VARS) -I$(UFO_LOG_PATH)/src -I$(UFO_LOG_PATH)/include -O3 -fmessage-length=0 -fvisibility=hidden -fPIC -DUFO_DYNLIB_COMPILE

override CXXFLAGS += $(FIXED_COMPILE_FLAGS)

override LDFLAGS += -Wl,--exclude-libs,ALL -shared

STAGE = stage

LIB = ./lib

INCLUDE = ./include

.PHONY: all clean

all : post-build
	ln -sf libufo-log.so.$(UFO_VERSION) $(LIB)/libufo-log.so
	
pre-build : 
	mkdir -p stage
	mkdir -p $(LIB)
	ln -sf ../$(SRC_UFO_PATH)/frontend_def.hpp $(STAGE)/frontend_def.cpp;
	

post-build : so_file a_file
	cp -Rf $(INCLUDE_PATH) .

so_file: pre-build frontend_def.o
	$(CXX) -o $(LIB)/libufo-log.so.$(UFO_VERSION) $(STAGE)/frontend_def.o $(LDLIBS) $(LDFLAGS)

a_file: pre-build frontend_def.o
	$(AR) rcs $(LIB)/libufo-log.a.$(UFO_VERSION) $(STAGE)/frontend_def.o

frontend_def.o: $(STAGE)/frontend_def.cpp
	$(CXX) $(CXXFLAGS) -c $(STAGE)/frontend_def.cpp -o $(STAGE)/frontend_def.o

clean:
	rm -rf lib stage include


