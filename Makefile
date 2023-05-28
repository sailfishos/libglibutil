# -*- Mode: makefile-gmake -*-

.PHONY: clean all debug release coverage pkgconfig install install-dev test
.PHONY: print_debug_lib print_release_lib print_coverage_lib
#
# Required packages
#

PKGS = glib-2.0 gobject-2.0

#
# Default target
#

all: debug release pkgconfig

#
# Directories
#

SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
DEBUG_BUILD_DIR = $(BUILD_DIR)/debug
RELEASE_BUILD_DIR = $(BUILD_DIR)/release
COVERAGE_BUILD_DIR = $(BUILD_DIR)/coverage

#
# Library version
#

VERSION_FILE = $(INCLUDE_DIR)/gutil_version.h
get_version = $(shell grep -E '^ *\#define +GUTIL_VERSION_$1 +[0-9]+$$' $(VERSION_FILE) | sed 's/  */ /g' | cut -d ' ' -f 3)

VERSION_MAJOR := $(call get_version,MAJOR)
VERSION_MINOR := $(call get_version,MINOR)
VERSION_MICRO := $(call get_version,MICRO)

# Version for pkg-config
PCVERSION := $(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_MICRO)

#
# Library name
#

NAME = glibutil
LIB_NAME = lib$(NAME)
LIB_DEV_SYMLINK = $(LIB_NAME).so
LIB_SYMLINK1 = $(LIB_DEV_SYMLINK).$(VERSION_MAJOR)
LIB_SYMLINK2 = $(LIB_SYMLINK1).$(VERSION_MINOR)
LIB_SONAME = $(LIB_SYMLINK1)
LIB = $(LIB_SONAME).$(VERSION_MINOR).$(VERSION_MICRO)
STATIC_LIB = $(LIB_NAME).a

#
# Sources
#

SRC = \
  gutil_datapack.c \
  gutil_history.c \
  gutil_idlepool.c \
  gutil_idlequeue.c \
  gutil_inotify.c \
  gutil_intarray.c \
  gutil_ints.c \
  gutil_log.c \
  gutil_misc.c \
  gutil_ring.c \
  gutil_strv.c \
  gutil_timenotify.c \
  gutil_objv.c \
  gutil_version.c \
  gutil_weakref.c

#
# Tools and flags
#

CC ?= $(CROSS_COMPILE)gcc
STRIP ?= strip
LD = $(CC)
WARNINGS = -Wall
INCLUDES = -I$(INCLUDE_DIR)
BASE_FLAGS = -fPIC
FULL_CFLAGS = $(BASE_FLAGS) $(CFLAGS) $(DEFINES) $(WARNINGS) $(INCLUDES) \
  -DGLIB_VERSION_MAX_ALLOWED=GLIB_VERSION_2_32 \
  -DGLIB_VERSION_MIN_REQUIRED=GLIB_VERSION_MAX_ALLOWED \
  -MMD -MP $(shell pkg-config --cflags $(PKGS))
FULL_LDFLAGS = $(BASE_FLAGS) $(LDFLAGS) -shared -Wl,-soname,$(LIB_SONAME) \
  $(shell pkg-config --libs $(PKGS))
DEBUG_FLAGS = -g
RELEASE_FLAGS =
COVERAGE_FLAGS = -g

KEEP_SYMBOLS ?= 0
ifneq ($(KEEP_SYMBOLS),0)
RELEASE_FLAGS += -g
endif

DEBUG_LDFLAGS = $(FULL_LDFLAGS) $(DEBUG_FLAGS)
RELEASE_LDFLAGS = $(FULL_LDFLAGS) $(RELEASE_FLAGS)
DEBUG_CFLAGS = $(FULL_CFLAGS) $(DEBUG_FLAGS) -DDEBUG
RELEASE_CFLAGS = $(FULL_CFLAGS) $(RELEASE_FLAGS) -O2
COVERAGE_CFLAGS = $(FULL_CFLAGS) $(COVERAGE_FLAGS) -O0 --coverage

#
# Files
#

PKGCONFIG = $(BUILD_DIR)/$(LIB_NAME).pc
DEBUG_OBJS = $(SRC:%.c=$(DEBUG_BUILD_DIR)/%.o)
RELEASE_OBJS = $(SRC:%.c=$(RELEASE_BUILD_DIR)/%.o)
COVERAGE_OBJS = $(SRC:%.c=$(COVERAGE_BUILD_DIR)/%.o)

DEBUG_LIB = $(DEBUG_BUILD_DIR)/$(LIB)
RELEASE_LIB = $(RELEASE_BUILD_DIR)/$(LIB)
DEBUG_LINK = $(DEBUG_BUILD_DIR)/$(LIB_SYMLINK1)
RELEASE_LINK = $(RELEASE_BUILD_DIR)/$(LIB_SYMLINK1)
DEBUG_DEV_LINK = $(DEBUG_BUILD_DIR)/$(LIB_DEV_SYMLINK)
RELEASE_DEV_LINK = $(RELEASE_BUILD_DIR)/$(LIB_DEV_SYMLINK)
DEBUG_STATIC_LIB = $(DEBUG_BUILD_DIR)/$(STATIC_LIB)
RELEASE_STATIC_LIB = $(RELEASE_BUILD_DIR)/$(STATIC_LIB)
COVERAGE_STATIC_LIB = $(COVERAGE_BUILD_DIR)/$(STATIC_LIB)

#
# Dependencies
#

DEPS = $(DEBUG_OBJS:%.o=%.d) $(RELEASE_OBJS:%.o=%.d) $(COVERAGE_OBJS:%.o=%.d)
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(DEPS)),)
-include $(DEPS)
endif
endif

$(PKGCONFIG): | $(BUILD_DIR)
$(DEBUG_OBJS) $(DEBUG_LIB) $(DEBUG_STATIC_LIB): | $(DEBUG_BUILD_DIR)
$(RELEASE_OBJS) $(RELEASE_LIB) $(RELEASE_STATIC_LIB): | $(RELEASE_BUILD_DIR)
$(COVERAGE_OBJS) $(COVERAGE_STATIC_LIB): | $(COVERAGE_BUILD_DIR)

#
# Rules
#

debug: $(DEBUG_STATIC_LIB) $(DEBUG_DEV_LINK)

release: $(RELEASE_STATIC_LIB) $(RELEASE_DEV_LINK)

coverage: $(COVERAGE_STATIC_LIB)

pkgconfig: $(PKGCONFIG)

print_debug_lib:
	@echo $(DEBUG_STATIC_LIB)

print_release_lib:
	@echo $(RELEASE_STATIC_LIB)

print_coverage_lib:
	@echo $(COVERAGE_STATIC_LIB)

clean:
	make -C test clean
	rm -fr test/coverage/results test/coverage/*.gcov
	rm -f *~ $(SRC_DIR)/*~ $(INCLUDE_DIR)/*~
	rm -fr $(BUILD_DIR) RPMS installroot
	rm -fr debian/tmp debian/libglibutil debian/libglibutil-dev
	rm -f documentation.list debian/files debian/*.substvars
	rm -f debian/*.debhelper.log debian/*.debhelper debian/*~
	rm -fr debian/*.install

test:
	make -C test test

$(BUILD_DIR):
	mkdir -p $@

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

$(DEBUG_LIB): $(DEBUG_OBJS)
	$(LD) $(DEBUG_OBJS) $(DEBUG_LDFLAGS) -o $@

$(RELEASE_LIB): $(RELEASE_OBJS)
	$(LD) $(RELEASE_OBJS) $(RELEASE_LDFLAGS) -o $@
ifeq ($(KEEP_SYMBOLS),0)
	$(STRIP) $@
endif

$(DEBUG_LINK): $(DEBUG_LIB)
	ln -sf $(LIB) $@

$(RELEASE_LINK): $(RELEASE_LIB)
	ln -sf $(LIB) $@

$(DEBUG_DEV_LINK): $(DEBUG_LINK)
	ln -sf $(LIB_SYMLINK1) $@

$(RELEASE_DEV_LINK): $(RELEASE_LINK)
	ln -sf $(LIB_SYMLINK1) $@

$(DEBUG_STATIC_LIB): $(DEBUG_OBJS)
	$(AR) rc $@ $?

$(RELEASE_STATIC_LIB): $(RELEASE_OBJS)
	$(AR) rc $@ $?

$(COVERAGE_STATIC_LIB): $(COVERAGE_OBJS)
	$(AR) rc $@ $?

#
# LIBDIR usually gets substituted with arch specific dir.
# It's relative in deb build and can be whatever in rpm build.
#

LIBDIR ?= usr/lib
ABS_LIBDIR := $(shell echo /$(LIBDIR) | sed -r 's|/+|/|g')

$(PKGCONFIG): $(LIB_NAME).pc.in Makefile
	sed -e 's|@version@|$(PCVERSION)|g' -e 's|@libdir@|$(ABS_LIBDIR)|g' $< > $@

debian/%.install: debian/%.install.in
	sed 's|@LIBDIR@|$(LIBDIR)|g' $< > $@

#
# Install
#

INSTALL_PERM  = 644

INSTALL = install
INSTALL_DIRS = $(INSTALL) -d
INSTALL_FILES = $(INSTALL) -m $(INSTALL_PERM)

INSTALL_LIB_DIR = $(DESTDIR)$(ABS_LIBDIR)
INSTALL_INCLUDE_DIR = $(DESTDIR)/usr/include/gutil
INSTALL_PKGCONFIG_DIR = $(DESTDIR)$(ABS_LIBDIR)/pkgconfig

#
# rpm based systems expect libraries to be 755
# deb based systems expect libraries to be 644
#
# 755 works for both because dh_fixperms removes execute permissions
# from any libraries and other files that don't need it.
#
install: $(INSTALL_LIB_DIR)
	$(INSTALL) -m 755 $(RELEASE_LIB) $(INSTALL_LIB_DIR)
	ln -sf $(LIB) $(INSTALL_LIB_DIR)/$(LIB_SYMLINK2)
	ln -sf $(LIB_SYMLINK2) $(INSTALL_LIB_DIR)/$(LIB_SYMLINK1)

install-dev: install $(INSTALL_INCLUDE_DIR) $(INSTALL_PKGCONFIG_DIR)
	$(INSTALL_FILES) $(INCLUDE_DIR)/*.h $(INSTALL_INCLUDE_DIR)
	$(INSTALL_FILES) $(PKGCONFIG) $(INSTALL_PKGCONFIG_DIR)
	ln -sf $(LIB_SYMLINK1) $(INSTALL_LIB_DIR)/$(LIB_DEV_SYMLINK)

$(INSTALL_LIB_DIR):
	$(INSTALL_DIRS) $@

$(INSTALL_INCLUDE_DIR):
	$(INSTALL_DIRS) $@

$(INSTALL_PKGCONFIG_DIR):
	$(INSTALL_DIRS) $@
