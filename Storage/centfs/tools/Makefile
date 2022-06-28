TOP := ../../..
include ../centfs-common.mk

PROGS := centfs-ls centfs-extract centboot
SRCS := $(addsuffix .c, $(PROGS))
LIST := $(addprefix $(TOOLBIN)/, $(PROGS))
EXES := $(addsuffix $(EXE_EXT), $(LIST))
SCRIPTS :=
EXES += $(addprefix $(TOOLBIN)/, $(SCRIPTS))

LIBS += $(COMMON_LIB_DIR)/libcommon-cent$(LIBA_EXT)
.PHONY: all clean $(PROGS)


all: $(EXES)

$(TOOLBIN)/%.sh: %.sh | $(TOOLBIN)
	rm -f "$@"
	cp "$<" "$@"
	chmod 0755 "$@"

$(TOOLBIN)/%$(EXE_EXT): obj/%.o | $(TOOLBIN)
#	$(LINK.o) $< $(CFLAGS) $(LDFLAGS) $(LDLIBS) -o "$@"
	$(CC) $(INC) $< $(CFLAGS) $(LDFLAGS) -o $@ $(LIBS)


clean:
	rm -f *.o $(EXES)


include ../centfs-end-common.mk
