# -*- Mode: makefile-gmake -*-

.PHONY: clean all debug release coverage debug_lib release_lib coverage_lib

#
# Real test makefile defines EXE (and possibly SRC) and includes this one.
#

ifndef EXE
${error EXE not defined}
endif

SRC ?= $(EXE).c
COMMON_SRC ?= test_main.c test_object.c

#
# Required packages
#

PKGS += gobject-2.0 glib-2.0

#
# Default target
#

all: debug release

#
# Directories
#

SRC_DIR = .
LIB_DIR = ../..
COMMON_DIR = ../common
BUILD_DIR = build
DEBUG_BUILD_DIR = $(BUILD_DIR)/debug
RELEASE_BUILD_DIR = $(BUILD_DIR)/release
COVERAGE_BUILD_DIR = $(BUILD_DIR)/coverage

#
# Tools and flags
#

CC ?= $(CROSS_COMPILE)gcc
LD = $(CC)
WARNINGS = -Wall
INCLUDES = -I$(LIB_DIR)/include -I$(LIB_DIR)/src -I$(COMMON_DIR)
BASE_FLAGS = -fPIC
BASE_LDFLAGS = $(BASE_FLAGS) $(LDFLAGS)
BASE_CFLAGS = $(BASE_FLAGS) $(CFLAGS)
FULL_CFLAGS = $(BASE_CFLAGS) $(DEFINES) $(WARNINGS) $(INCLUDES) -MMD -MP \
  -DGLIB_VERSION_MAX_ALLOWED=GLIB_VERSION_2_32 \
  -DGLIB_VERSION_MIN_REQUIRED=GLIB_VERSION_MAX_ALLOWED \
  $(shell pkg-config --cflags $(PKGS))
FULL_LDFLAGS = $(BASE_LDFLAGS)
LIBS = $(shell pkg-config --libs $(PKGS))
QUIET_MAKE = $(MAKE) --no-print-directory
DEBUG_FLAGS = -g
RELEASE_FLAGS =
COVERAGE_FLAGS = -g

KEEP_SYMBOLS ?= 0
ifneq ($(KEEP_SYMBOLS),0)
RELEASE_FLAGS += -g
SUBMAKE_OPTS += KEEP_SYMBOLS=1
endif

DEBUG_LDFLAGS = $(FULL_LDFLAGS) $(DEBUG_FLAGS)
RELEASE_LDFLAGS = $(FULL_LDFLAGS) $(RELEASE_FLAGS)
COVERAGE_LDFLAGS = $(FULL_LDFLAGS) $(COVERAGE_FLAGS) --coverage

DEBUG_CFLAGS = $(FULL_CFLAGS) $(DEBUG_FLAGS) -DDEBUG
RELEASE_CFLAGS = $(FULL_CFLAGS) $(RELEASE_FLAGS) -O2
COVERAGE_CFLAGS = $(FULL_CFLAGS) $(COVERAGE_FLAGS) --coverage

#
# Files
#

DEBUG_OBJS = \
  $(COMMON_SRC:%.c=$(DEBUG_BUILD_DIR)/common_%.o) \
  $(SRC:%.c=$(DEBUG_BUILD_DIR)/%.o)
RELEASE_OBJS = \
  $(COMMON_SRC:%.c=$(RELEASE_BUILD_DIR)/common_%.o) \
  $(SRC:%.c=$(RELEASE_BUILD_DIR)/%.o)
COVERAGE_OBJS = \
  $(COMMON_SRC:%.c=$(COVERAGE_BUILD_DIR)/common_%.o) \
  $(SRC:%.c=$(COVERAGE_BUILD_DIR)/%.o)

DEBUG_LIB_FILE := $(shell $(QUIET_MAKE) -C $(LIB_DIR) print_debug_lib)
RELEASE_LIB_FILE := $(shell $(QUIET_MAKE) -C $(LIB_DIR) print_release_lib)
COVERAGE_LIB_FILE := $(shell $(QUIET_MAKE) -C $(LIB_DIR) print_coverage_lib)

DEBUG_LIB := $(LIB_DIR)/$(DEBUG_LIB_FILE)
RELEASE_LIB := $(LIB_DIR)/$(RELEASE_LIB_FILE)
COVERAGE_LIB := $(LIB_DIR)/$(COVERAGE_LIB_FILE)

#
# Dependencies
#

DEPS = $(DEBUG_OBJS:%.o=%.d) $(RELEASE_OBJS:%.o=%.d) $(COVERAGE_OBJS:%.o=%.d)
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(DEPS)),)
-include $(DEPS)
endif
endif

$(DEBUG_LIB): | debug_lib
$(RELEASE_LIB): | release_lib
$(COVERAGE_LIB): | coverage_lib

$(DEBUG_OBJS): | $(DEBUG_BUILD_DIR)
$(RELEASE_OBJS): | $(RELEASE_BUILD_DIR)
$(COVERAGE_OBJS): | $(COVERAGE_BUILD_DIR)

#
# Rules
#

DEBUG_EXE = $(DEBUG_BUILD_DIR)/$(EXE)
RELEASE_EXE = $(RELEASE_BUILD_DIR)/$(EXE)
COVERAGE_EXE = $(COVERAGE_BUILD_DIR)/$(EXE)

debug: debug_lib $(DEBUG_EXE)

release: release_lib $(RELEASE_EXE)

coverage: coverage_lib $(COVERAGE_EXE)

clean:
	rm -f *~
	rm -fr $(BUILD_DIR)

cleaner: clean
	@$(MAKE) -C $(LIB_DIR) clean

test_banner:
	@echo "===========" $(EXE) "=========== "

test: test_banner debug 
	@$(DEBUG_EXE)

valgrind: test_banner debug
	@G_DEBUG=gc-friendly G_SLICE=always-malloc valgrind --tool=memcheck --leak-check=full --show-possibly-lost=no $(DEBUG_EXE)

$(DEBUG_BUILD_DIR):
	mkdir -p $@

$(RELEASE_BUILD_DIR):
	mkdir -p $@

$(COVERAGE_BUILD_DIR):
	mkdir -p $@

$(DEBUG_BUILD_DIR)/%.o : $(SRC_DIR)/%.c
	$(CC) -c $(DEBUG_CFLAGS) -MT"$@" -MF"$(@:%.o=%.d)" $< -o $@

$(RELEASE_BUILD_DIR)/%.o : $(SRC_DIR)/%.c
	$(CC) -c $(RELEASE_CFLAGS) -MT"$@" -MF"$(@:%.o=%.d)" $< -o $@

$(COVERAGE_BUILD_DIR)/%.o : $(SRC_DIR)/%.c
	$(CC) -c $(COVERAGE_CFLAGS) -MT"$@" -MF"$(@:%.o=%.d)" $< -o $@

$(DEBUG_BUILD_DIR)/common_%.o : $(COMMON_DIR)/%.c
	$(CC) -c $(DEBUG_CFLAGS) -MT"$@" -MF"$(@:%.o=%.d)" $< -o $@

$(RELEASE_BUILD_DIR)/common_%.o : $(COMMON_DIR)/%.c
	$(CC) -c $(RELEASE_CFLAGS) -MT"$@" -MF"$(@:%.o=%.d)" $< -o $@

$(COVERAGE_BUILD_DIR)/common_%.o : $(COMMON_DIR)/%.c
	$(CC) -c $(COVERAGE_CFLAGS) -MT"$@" -MF"$(@:%.o=%.d)" $< -o $@

$(DEBUG_EXE): $(DEBUG_LIB) $(DEBUG_BUILD_DIR) $(DEBUG_OBJS)
	$(LD) $(DEBUG_LDFLAGS) $(DEBUG_OBJS) $< $(LIBS) -o $@

$(RELEASE_EXE): $(RELEASE_LIB) $(RELEASE_BUILD_DIR) $(RELEASE_OBJS)
	$(LD) $(RELEASE_LDFLAGS) $(RELEASE_OBJS) $< $(LIBS) -o $@
ifeq ($(KEEP_SYMBOLS),0)
	strip $@
endif

$(COVERAGE_EXE): $(COVERAGE_LIB) $(COVERAGE_BUILD_DIR) $(COVERAGE_OBJS)
	$(LD) $(COVERAGE_LDFLAGS) $(COVERAGE_OBJS) $< $(LIBS) -o $@

debug_lib:
	@$(MAKE) $(SUBMAKE_OPTS) -C $(LIB_DIR) debug

release_lib:
	@$(MAKE) $(SUBMAKE_OPTS) -C $(LIB_DIR) release

coverage_lib:
	@$(MAKE) $(SUBMAKE_OPTS) -C $(LIB_DIR) coverage
