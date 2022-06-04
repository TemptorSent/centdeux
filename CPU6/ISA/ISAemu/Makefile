TOP := ../../..
include ../CPU6-ISA-common.mk

CPU6_ISAemu_TARGET := CPU6-ISAemu
CPU6_ISAemu_BIN := $(BIN)/$(CPU6_ISAemu_TARGET)$(EXE_EXT)
CPU6_ISAemu_SRCS := CPU6-ISAemu.c

SRCS = $(CPU6_ISAemu_SRCS)
TARGETS = $(CPU6_ISAemu_TARGET)
BINS = $(CPU6_ISAemu_BIN)

.PHONY: all clean $(TARGETS)

all: $(TARGETS)

$(CPU6_ISAemu_TARGET): $(CPU6_ISAemu_BIN)
	printf "Building %s\n" $@

$(CPU6_ISAemu_BIN): $(CPU6_ISAemu_SRCS:%.c=$(OBJDIR)/%.o)
	$(LINK.o) $^ $(LDLIBS) -o $@


clean:
	rm -f *.o obj/*.o $(BINS)

include ../CPU6-ISA-end-common.mk
