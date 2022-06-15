TOP := ../../..
include ../DSK-common.mk

PROGS := hawk-CHS-hexdump
SRCS := $(addsuffix .c, $(PROGS))
LIST := $(addprefix $(TOOLBIN)/, $(PROGS))
EXES := $(addsuffix $(EXE_EXT), $(LIST))
SCRIPTS := od_hawk-CHS.sh
EXES += $(addprefix $(TOOLBIN)/, $(SCRIPTS))
.PHONY: all clean $(PROGS) $(SCRIPTS)


all: $(EXES)

$(TOOLBIN)/%.sh: %.sh | $(TOOLBIN)
	rm -f "$@"
	cp "$<" "$@"
	chmod 0755 "$@"

$(TOOLBIN)/%$(EXE_EXT): obj/%.o | $(TOOLBIN)
	$(CC) $(INC) $< $(CFLAGS) -o $@ $(LIBS)


clean:
	rm -f *.o $(EXES)


include ../DSK-end-common.mk
