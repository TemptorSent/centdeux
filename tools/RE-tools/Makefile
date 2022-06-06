TOP := ../..
include $(TOP)/common/common.mk

PROGS := stripMSB
SRCS := $(addsuffix .c, $(PROGS))
LIST := $(addprefix $(TOOLBIN)/, $(PROGS))
EXES := $(addsuffix $(EXE_EXT), $(LIST))
SCRIPTS := centstrings.sh
EXES += $(addprefix $(TOOLBIN)/, $(SCRIPTS))
.PHONY: all clean $(PROGS)


all: $(EXES) 

$(TOOLBIN)/%.sh: %.sh
	rm -f "$@"
	cp "$<" "$@"

$(TOOLBIN)/%$(EXE_EXT): obj/%.o
	$(CC) $(INC) $< $(CFLAGS) -o $@ $(LIBS)


clean:
	rm -f *.o $(EXES)


include $(TOP)/common/end-common.mk
