TOP := ../../..
include ../CPU6-uCode-common.mk

CPU6_uCemu_TARGET := CPU6-uCemu
CPU6_uCemu_BIN := $(BIN)/$(CPU6_uCemu_TARGET)$(EXE_EXT)
CPU6_uCemu_SRCS := CPU6-uCemu.c uI.c uProg.c

SRCS = $(CPU6_uCemu_SRCS)
TARGETS = $(CPU6_uCemu_TARGET)
BINS = $(CPU6_uCemu_BIN)

.PHONY: all clean $(TARGETS)

all: $(TARGETS)

$(CPU6_uCemu_TARGET): $(CPU6_uCemu_BIN)
	printf "Building %s\n" $@

$(CPU6_uCemu_BIN): $(CPU6_uCemu_SRCS:%.c=$(OBJDIR)/%.o) | $(BIN)
	$(LINK.o) $^ $(LDLIBS) -o $@


clean:
	rm -f *.o obj/*.o $(BINS)

include ../CPU6-uCode-end-common.mk
