TOP := ../../..
include $(TOP)/common/common.mk


liba_core_TARGET = libALU_am2901$(LIBA_EXT)

liba_core_SRCS := ALU_am2901_core.c
test_core_SRCS := test_ALU_am2901_core.c
SRCS = $(liba_core_SRCS) $(test_SRCS)

test_PROGS := test_ALU_am2901_core
test_EXES := $(addsuffix $(EXE_EXT), $(test_PROGS))
.PHONY: all clean test


all: $(liba_core_TARGET)


$(liba_core_TARGET): $(liba_core_SRCS:%.c=$(OBJDIR)/%.o)
	$(AR) -rcsv $@ $^

test_ALU_am2901_core$(EXE_EXT): $(test_core_SRCS:%.c=$(OBJDIR)/%.o) $(liba_core_TARGET)
	$(CC) $^ $(CFLAGS) -o $@ $(LIBS)

test: $(test_EXES)
	./test_ALU_am2901_core$(EXE_EXT)
clean:
	rm -f $(liba_TARGET)
	rm -f *.o *.a *.lib *.so .*dll $(test_EXES) 
	rm -f $(OBJDIR)/*.o


include $(TOP)/common/end-common.mk

