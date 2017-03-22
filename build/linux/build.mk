################################################################################
# Input variables for the parent Makefile
#
#
# -ARTIFACT: Name of the generated artifact, in the form of "bin/foo" or
#   "lib/libbar". The binary type is deduced from the bin/lib prefix.
#
# -BUILD_DIR: Name of the directory where the out-of-tree build objects will be
#    stored in.
#
# -TOP: Absolute folder from where the source dirs and "include" can be
#    accessed.
#
# -SRC_DIRS: Space delimited source directories that will be recursively built.
#    They have to be under $(TOP). Defaults to $(TOP)/src
#
# -INCLUDE_DIRS: Space delimited include directories that will be recursively
#    added to the compiler flats. They have to be under $(TOP). Defaults to
#    $(TOP)/include
#
# -GIT_SUBMODULES_DIR: If this folder exists the modules will be automatically
#    updated before building. This is defaulted to $(TOP)/git-submodules
#
# -EXTRA_TARGET_DEPS: Extra dependencies to the binary targets.
#
# -LD: Has to be set to CXX for C++.
#
# Input variables meant to be overriden from the "make" command call or the
# environment.
#
# Many variables are undocumented here, as there will be seldomly needed. Try to
# search for the "?=" to see what can be overriden.
#
# -DEBUG (0,1)
#
# -VERBOSE (0,1)
#
# -Standard build variables: CC, CXX, AR, CFLAGS, CXXFLAGS, CPPFLAGS, KCFLAGS,
#    LDLIBS, LDFLAGS, AR. KCXXFLAGS is used as a KCFLAGS equivalent for C++ but
#    I'm not sure if it's standard.
#
# -Standard install variables: DESTDIR, prefix, exec_prefix, libdir, includedir
#    bindir.
#
################################################################################

## Folders ---------------------------------------------------------------------
TOP          ?= $$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
SRC_DIRS     ?= $(TOP)/src
INCLUDE_DIRS ?= $(TOP)/include

## Debug/Release setup ---------------------------------------------------------
DEBUG ?= 0
ifeq ($(DEBUG), 0)
	CPPFLAGS      += -DNDEBUG
	CCCXXFLAGS    += -Os #CCCXXFLAGS flags are both for CC and CXX
	BUILDTYPE     :=
	DEBUG_SUFFIX  :=
	INSTALL_STRIP :=
else
	CPPFLAGS      += -DDEBUG
	CCCXXFLAGS    += -g #CCCXXFLAGS flags are both for CC and CXX
	CCXXFLAGS     += -g
	BUILDTYPE     := .debug
	DEBUG_SUFFIX  := .d
	INSTALL_STRIP := -s
endif
## Library/executable setup ----------------------------------------------------
ARTIFACT_DIR  := $(shell dirname $(ARTIFACT))
ARTIFACT_NAME := $(shell basename $(ARTIFACT))

ifeq ($(ARTIFACT_DIR),lib)
	IS_LIB                   := 1
	EXTENSION                := .so
	ARTIFACT_NO_SYMLINK      ?= 0
	ARTIFACT_NO_DEBUG_SUFFIX ?= 0
	ARTIFACT_STATLIB         ?= 1
	LDFLAGS                  += -Wl,--exclude-libs,ALL -shared
	CCCXXFLAGS               += -fvisibility=hidden -fPIC
	INSTALL_STRIP            :=
else ifeq ($(ARTIFACT_DIR),bin)
	IS_LIB                   := 0
	EXTENSION                :=
	ARTIFACT_NO_SYMLINK      ?= 1
	ARTIFACT_NO_DEBUG_SUFFIX ?= 1
	ARTIFACT_STATLIB         ?= 0
else
	$(error Unknown artifact type)
endif

## Out of tree build and stage dirs --------------------------------------------
BUILD_DIR   ?= $(TOP)/build
TARGET_MACH := $(shell $(CC) -dumpmachine)

OBJ  := $(BUILD_DIR)/obj/$(TARGET_MACH)$(BUILDTYPE)
STAGE:= $(BUILD_DIR)/stage/$(TARGET_MACH)$(BUILDTYPE)

LD ?= $(CC)
## Other variables -------------------------------------------------------------
VERSION_MINOR ?= 0
VERSION_REV   ?= 0
ifneq ($(VERSION_MAJOR), )
	VERSION := .$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_REV)
endif
VERBOSE ?= 0
ifeq ($(VERBOSE), 1)
	CMDPRINT :=
else
	CMDPRINT := @
endif
STATLIB_NO_VER_SYMLINK ?=
INSTALLCMD    ?= install -m 755

## Default fixed compiler + linker flags ---------------------------------------
CCCXXFLAGS += $(foreach SRC_DIR,$(SRC_DIRS),-I$(SRC_DIR))
CCCXXFLAGS += $(foreach INCLUDE_DIR,$(INCLUDE_DIRS),-I$(INCLUDE_DIR))
LDFLAGS    += -L$(STAGE)/lib

CFLAGS     += $(CCCXXFLAGS)
CXXFLAGS   += $(CCCXXFLAGS)

## Standard GNU make install destinations --------------------------------------
prefix ?= /usr/local
ifneq ($(PREFIX), )
	prefix := $(PREFIX)
endif

exec_prefix ?= $(prefix)
bindir      ?= $(exec_prefix)/bin
libdir      ?= $(exec_prefix)/lib
includedir  ?= $(exec_prefix)/include

TARGET_BIN     := $(DESTDIR)$(bindir)
TARGET_LIB     := $(DESTDIR)$(libdir)
TARGET_INCLUDE := $(DESTDIR)$(includedir)

ARTIFACT_STATLIB_NO_SYMLINK      ?= 1
ARTIFACT_STATLIB_NO_DEBUG_SUFFIX ?= 1

ifneq ($(ARTIFACT_STATLIB_NO_DEBUG_SUFFIX), 1)
	STATLIB_DEBUG_SUFFIX := $(DEBUG_SUFFIX)
endif

ifeq ($(ARTIFACT_NO_DEBUG_SUFFIX), 1)
	DEBUG_SUFFIX :=
endif

ifeq ($(IS_LIB), 1)
	TARGETDIR := $(TARGET_LIB)
else
	TARGETDIR := $(TARGET_BIN)
endif

## Artifact naming -------------------------------------------------------------
ARTIFACT_NAME_VER   := $(ARTIFACT_NAME)$(EXTENSION)$(VERSION)$(DEBUG_SUFFIX)
ARTIFACT_NAME_NOVER := $(ARTIFACT_NAME)$(EXTENSION)$(DEBUG_SUFFIX)
ARTIFACT_STATLIB_NAME_VER   := $(ARTIFACT_NAME).a$(VERSION)$(STATLIB_DEBUG_SUFFIX)
ARTIFACT_STATLIB_NAME_NOVER := $(ARTIFACT_NAME).a$(STATLIB_DEBUG_SUFFIX)

## Object files ----------------------------------------------------------------
# C
CC_SRCS     := $(shell find $(SRC_DIRS) -type f -name '*.c')
CC_OBJS     := $(patsubst $(TOP)/%.c, $(OBJ)/%.c.o, $(CC_SRCS))
CC_HDR_DEPS := $(CC_OBJS:%.o=%.d)

# C++
CXX_EXT      := cpp cxx cc C c++ CPP cp
GREP_ARGS    := $(foreach EXT, $(CXX_EXT), -e .${EXT}$$)
CXX_SRCS     := $(shell find $(SRC_DIRS) -type f | grep  ${GREP_ARGS})
CXX_OBJS     := $(patsubst $(TOP)/%, $(OBJ)/%.o, $(CXX_SRCS))
CXX_HDR_DEPS := $(CXX_OBJS:%.o=%.d)

OBJFILES := $(CC_OBJS) $(CXX_OBJS)
HDR_DEPS := $(CC_HDR_DEPS) $(CXX_HDR_DEPS)

## Submodules ------------------------------------------------------------------
GIT_SUBMODULES_DIR ?= $(TOP)/git-submodules

## Targets ---------------------------------------------------------------------
MAIN_TARGET    := $(STAGE)/$(ARTIFACT_DIR)/$(ARTIFACT_NAME_VER)
STATLIB_TARGET := $(STAGE)/lib/$(ARTIFACT_STATLIB_NAME_VER)

ifneq ($(wildcard $(GIT_SUBMODULES_DIR)/.),)
	TARGETS += update-gitmodules
endif
TARGETS += $(MAIN_TARGET)
ifeq ($(ARTIFACT_STATLIB), 1)
	TARGETS += $(STATLIB_TARGET)
endif

## Recipes ---------------------------------------------------------------------
all: $(TARGETS)

update-gitmodules:
	@echo "GIT Syncing submodules"
	$(CMDPRINT)git submodule update --init --recursive

$(MAIN_TARGET): $(EXTRA_TARGET_DEPS) $(OBJFILES)
	@echo "LD $(OBJFILES:$(OBJ)/%=%)"
	$(CMDPRINT)mkdir -p $(STAGE)/$(ARTIFACT_DIR)
	$(CMDPRINT)$(LD) -o $(MAIN_TARGET) $(OBJFILES) $(LDLIBS) $(LDFLAGS)

$(STATLIB_TARGET): $(EXTRA_TARGET_DEPS) $(OBJFILES)
	@echo "AR $(OBJFILES:$(OBJ)/%=%)"
	$(CMDPRINT)mkdir -p $(STAGE)/lib
	$(CMDPRINT)$(AR) rcs $(STATLIB_TARGET) $(OBJFILES)

-include $(HDR_DEPS)

#C
$(OBJ)%.c.o: $(TOP)%.c
	@echo CC $(<:$(TOP)/%=%)
	$(CMDPRINT)mkdir -p "$(@D)"
	$(CMDPRINT)$(CC) -MMD -c $(CPPFLAGS) $(CFLAGS) $< -o $@ $(KCFLAGS)

#C++
define cpp_recipe_body
	@echo CXX $(<:$(TOP)/%=%)
	$(CMDPRINT)mkdir -p "$(@D)"
	$(CMDPRINT)$(CXX) -MMD -c $(CPPFLAGS) $(CXXFLAGS) $< -o $@ $(KCXXFLAGS)
endef
define cpp_recipe
$(OBJ)%.$1.o: $(TOP)%.$1
# The body is defined as a separate function because this function is called
# with "eval", which would required escaping every "$" symbol with an extra
# "$". Defining a function called without eval allows to just escape the
# function call invocation.
	$$(call cpp_recipe_body)
endef
# This requires a fairly new GNU make.
$(foreach EXT, $(CXX_EXT), $(eval $(call cpp_recipe,$(EXT))))

.PHONY: clean install

clean:
	rm -rf $(OBJ) $(STAGE) $(EXTRA_CLEAN_DIRS)

install:
# Main artifact  ---------------------------------------------------------------
	mkdir -p $(TARGETDIR)
	$(INSTALLCMD) $(INSTALL_STRIP) $(STAGE)/$(ARTIFACT_DIR)/$(ARTIFACT_NAME_VER) $(TARGETDIR)

ifeq ($(ARTIFACT_NO_SYMLINK), 1)
	mv $(TARGETDIR)/$(ARTIFACT_NAME_VER) $(TARGETDIR)/$(ARTIFACT_NAME_NOVER)
else
	ln -sf $(ARTIFACT_NAME_VER) $(ARTIFACT_NAME_NOVER)
	mv $(ARTIFACT_NAME_NOVER) $(TARGETDIR)
endif
# Static library ---------------------------------------------------------------
ifeq ($(ARTIFACT_STATLIB), 1)
	mkdir -p $(TARGET_LIB)
	$(INSTALLCMD) $(STAGE)/lib/$(ARTIFACT_STATLIB_NAME_VER) $(TARGET_LIB)

ifeq ($(ARTIFACT_STATLIB_NO_SYMLINK), 1)
	mv $(TARGETDIR)/$(ARTIFACT_STATLIB_NAME_VER) $(TARGETDIR)/$(ARTIFACT_STATLIB_NAME_NOVER)
else
	ln -sf $(ARTIFACT_STATLIB_NAME_VER) $(ARTIFACT_STATLIB_NAME_NOVER)
	mv $(ARTIFACT_STATLIB_NAME_NOVER) $(TARGETDIR)
endif
endif
# Includes ---------------------------------------------------------------------
ifeq ($(IS_LIB), 1)
	cp -rf  $(INCLUDE) $(TARGET_INCLUDE)
endif
