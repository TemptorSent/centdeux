TOP := ../../..
include $(TOP)/common/common.mk


liba_core_TARGET = libALU_am2901_core$(LIBA_EXT)

liba_core_SRCS := ALU_am2901_core.c
test_core_SRCS := test_ALU_am2901_core.c

core_INCS := ALU_am2901_core.h
INCS := $(core_INCS)
common_INCS := $(addprefix $(COMMON_INC_DIR)/, $(INCS))


test_core_PROGS := test_ALU_am2901_core
test_PROGS := $(test_core_PROGS)
test_SRCS := $(addsuffix .c, $(test_PROGS))
test_EXES := $(addsuffix $(EXE_EXT), $(test_PROGS))

SRCS = $(liba_core_SRCS) $(test_SRCS)

.PHONY: all clean test


all: $(liba_core_TARGET) $(common_INCS)


$(liba_core_TARGET): $(liba_core_SRCS:%.c=$(OBJDIR)/%.o)
	$(AR) -rcsv "$@" $(patsubst %, "%", $^)


$(COMMON_INC_DIR)/%.h: %.h | $(COMMON_INC_DIR)
	cp "$<" "$@"

$(test_EXES):%$(EXE_EXT): $(OBJDIR)/%.o $(liba_TARGET)
	$(CC) $(patsubst %, "%", $^) $(CFLAGS) -o "$@" $(LIBS)

test: $(test_EXES)
	$(patsubst %, "./%" ;, $^)
clean:
	rm -f $(patsubst %, "%", $(liba_TARGET) $(test_EXES))
	rm -f *.o *.a *.lib *.so .*dll
	rm -f $(patsubst %, "%", $(OBJDIR))/*.o

distclean: clean
	rm -f $(patsubst %, "%", $(common_INCS))


include $(TOP)/common/end-common.mk

