TOP := ../../..
include $(TOP)/common/common.mk

PROGS := repack_ccdp_dump
SRCS := $(addsuffix .c, $(PROGS))
LIST := $(addprefix $(TOOLBIN)/, $(PROGS))
EXES := $(addsuffix $(EXE_EXT), $(LIST))
SCRIPTS := od_centurionCHS.sh
EXES += $(addprefix $(TOOLBIN)/, $(SCRIPTS))
.PHONY: all clean $(PROGS)


all: $(EXES)

$(TOOLBIN)/%.sh: %.sh | $(TOOLBIN)
	rm -f "$@"
	cp "$<" "$@"
	chmod 0755 "$@"

$(TOOLBIN)/%$(EXE_EXT): obj/%.o | $(TOOLBIN)
	$(CC) $(INC) $< $(CFLAGS) -o $@ $(LIBS)


clean:
	rm -f *.o $(EXES)


include $(TOP)/common/end-common.mk
