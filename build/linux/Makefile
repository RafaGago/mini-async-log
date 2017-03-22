#Intermediate temporary variables
THIS_FILE_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

# Mandatory variables items
VERSION_MAJOR := 1
VERSION_MINOR := 0
VERSION_REV   := 1
ARTIFACT      ?= lib/libmal-log

# Standard directory layout overrides
TOP       := $(THIS_FILE_DIR)/../..
BUILD_DIR := $(THIS_FILE_DIR)/build

# Compiler setup
CPPFLAGS += -DMAL_DYNLIB_COMPILE
CXXFLAGS += -std=c++0x -fmessage-length=0
LD       := $(CXX)

include build.mk