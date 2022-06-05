TOP := ../../..
include ../CPU6-ISA-common.mk

CFLAGS += -fkeep-inline-functions -finline-limit=1200 -O2 -Winline
CPU6_ISAemu_TARGET := CPU6-ISAemu
CPU6_ISAemu_BIN := $(BIN)/$(CPU6_ISAemu_TARGET)$(EXE_EXT)
CPU6_ISAemu_SRCS := CPU6-ISAemu.c
CPU6_ISAemu_GEN_INC := ../include/$(CPU6_ISAemu_TARGET)_generated.h

SRCS = $(CPU6_ISAemu_SRCS)
TARGETS = $(CPU6_ISAemu_TARGET)
GEN_INCS = $(CPU6_ISAemu_GEN_INC)
BINS = $(CPU6_ISAemu_BIN)

.PHONY: all clean depclean $(TARGETS)

all: $(GEN_INCS) $(TARGETS) 


$(CPU6_ISAemu_GEN_INC): $(CPU6_ISAemu_SRCS)
	make -C $(dir $@) $(notdir $@)

$(CPU6_ISAemu_TARGET): $(CPU6_ISAemu_BIN)
	@:

$(CPU6_ISAemu_BIN): $(CPU6_ISAemu_SRCS:%.c=$(OBJDIR)/%.o)
	@printf "Building %s\n" $@
	$(LINK.o) $^ $(LDLIBS) -o $@


clean:
	rm -f *.o obj/*.o $(BINS)

depclean: clean
	rm -rf obj/

include ../CPU6-ISA-end-common.mk
