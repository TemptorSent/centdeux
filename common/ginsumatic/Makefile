TOP := ../..
include $(TOP)/common/common.mk

liba_TARGET = $(COMMON_LIB_DIR)/libginsumatic$(LIBA_EXT)
liba_INCS := $(COMMON_INC_DIR)/ginsumatic.h

liba_SRCS := ginsumatic.c
test_SRCS := test_ginsumatic.c


SRCS = $(liba_SRCS) $(test_SRCS)

test_PROGS := test_ginsumatic
test_EXES := $(addsuffix $(EXE_EXT), $(test_PROGS))
.PHONY: all clean test


all: $(liba_TARGET) $(liba_INCS)


$(liba_TARGET): $(liba_SRCS:%.c=$(OBJDIR)/%.o) | $(COMMON_LIB_DIR)
	$(AR) -rcsv "$@" "$^"

$(COMMON_INC_DIR)/%.h: %.h | $(COMMON_INC_DIR)
	cp "$<" "$@"

test_ginsumatic$(EXE_EXT): $(test_SRCS:%.c=$(OBJDIR)/%.o) $(liba_TARGET)
	$(CC) "$^" $(CFLAGS) -o "$@" $(LIBS)

test: $(test_EXES)
	./test_ginsumatic$(EXE_EXT)
clean:
	rm -f $(liba_TARGET)
	rm -f *.o *.a *.lib *.so .*dll $(test_EXES) 
	rm -f $(OBJDIR)/*.o


include $(TOP)/common/end-common.mk

