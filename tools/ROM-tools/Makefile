TOP := ../..
include $(TOP)/common/common.mk

PROGS := rom2include romcat
LIST := $(addprefix $(TOOLBIN)/, $(PROGS))
EXES := $(addsuffix $(EXE_EXT), $(LIST))
.PHONY: all clean $(PROGS)


all: $(PROGS)

rom2include: $(TOOLBIN)/rom2include$(EXE_EXT)
	@:

romcat: $(TOOLBIN)/romcat$(EXE_EXT)
	@:

$(TOOLBIN)/%$(EXE_EXT): %.c
	$(CC) $(INC) $< $(CFLAGS) -o $@ $(LIBS)


clean:
	rm -f *.o $(EXES)

