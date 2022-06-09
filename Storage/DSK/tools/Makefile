TOP := ../../..
include ../DSK-common.mk

PROGS := hawk-CHS-hexdump
SRCS := $(addsuffix .c, $(PROGS))
LIST := $(addprefix $(TOOLBIN)/, $(PROGS))
EXES := $(addsuffix $(EXE_EXT), $(LIST))
SCRIPTS :=  
EXES += $(addprefix $(TOOLBIN)/, $(SCRIPTS))
.PHONY: all clean $(PROGS)


all: $(EXES) 

$(TOOLBIN)/%.sh: %.sh
	rm -f "$@"
	cp "$<" "$@"
	chmod 0755 "$@"

$(TOOLBIN)/%$(EXE_EXT): obj/%.o
	$(CC) $(INC) $< $(CFLAGS) -o $@ $(LIBS)


clean:
	rm -f *.o $(EXES)


include ../DSK-end-common.mk
