
# Set our default C compiler to 'gcc' if the default 'cc' was not overridden
ifeq ($(origin CC),default)
CC = gcc
endif

# Set our CFLAGS to warn on everything except unused vars
CFLAGS ?= -Wall -Wno-unused -ggdb

# Set paths for top level binaries and tool binaries
BIN ?= $(TOP)/bin
TOOLBIN ?= $(TOP)/tools/bin

# Set path for top level common dir
COMMON_DIR = $(TOP)/common

# Add common includes to list of include dirs and add CFLAGS for them
INC_DIRS += $(COMMON_DIR)/include
INC_PARAMS = $(addprefix -I, $(INC_DIRS))
CFLAGS += $(INC_PARAMS)

# Detect our OS
ifeq ('$(findstring ;, $(PATH))', ';')
    uname_s := Windows
else
    uname_s := $(shell uname -s 2>/dev/null || echo Unknown)
    uname_s := $(patsubst CYGWIN%,Cygwin,$(uname_s))
    uname_s := $(patsubst MSYS%,MSYS,$(uname_s))
    uname_s := $(patsubst MINGW%,MSYS,$(uname_s))
endif

# Set our executable and library extensions
ifeq ($(uname_s), $(filter Windows Cygwin MSYS, $(uname_s)))
    EXE_EXT := .exe
    LIBA_EXT := .lib
    SHLIB_EXT := .dll
else
    EXE_EXT :=
    LIBA_EXT := .a
    SHLIB_EXT := .so
endif

# Handle generating deps files for target object files automagickly
# Don't forget to include the end-common.mk portion at the end of your Makefiles!
# Credit: http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/
OBJDIR := obj

DEPDIR := $(OBJDIR)/.deps
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d

COMPILE.c = $(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c

$(OBJDIR)/%.o : %.c $(DEPDIR)/%.d | $(DEPDIR)
	$(COMPILE.c) $(OUTPUT_OPTION) $<


