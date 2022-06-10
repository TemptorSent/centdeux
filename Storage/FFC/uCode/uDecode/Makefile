TOP := ../../..
include ../FFC-uCode-common.mk

PROGS := FFC-uDecode
LIST := $(addprefix $(BIN)/, $(PROGS))
EXES := $(addsuffix $(EXE_EXT), $(LIST))

.PHONY: all clean $(PROGS)

SRCS_FFC_uDecode = uDecode.c
LIB_ginsumatic = $(TOP)/common/ginsumatic/libginsumatic$(LIBA_EXT)
SRCS = $(SRCS_FFC_uDecode)

all: $(DEPDIR) $(PROGS)


FFC-uDecode: $(BIN)/FFC-uDecode$(EXE_EXT)
	@:

$(LIB_ginsumatic):
	make -C $(dir $@)


$(BIN)/FFC-uDecode$(EXE_EXT): $(SRCS_FFC_uDecode:%.c=obj/%.o) $(LIB_ginsumatic) | $(BIN)
	$(LINK.o) $^ $(LDLIBS) -o $@


clean:
	rm -f *.o $(EXES)

include ../FFC-uCode-end-common.mk
