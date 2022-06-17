TOP := ../..
include $(TOP)/common/common.mk



liba_TARGET = $(COMMON_LIB_DIR)/libcommon-cent$(LIBA_EXT)

liba_SRCS := common-cents.c

INCS := common-cents.h
common_INCS := $(addprefix $(COMMON_INC_DIR)/, $(INCS))

test_PROGS := test_common-cents
test_SRCS := $(addsuffix .c, $(test_PROGS))
test_EXES := $(addsuffix $(EXE_EXT), $(test_PROGS))

SRCS = $(liba_SRCS) $(test_SRCS)

.PHONY: all clean test distclean


all: $(liba_TARGET) $(common_INCS)


$(liba_TARGET): $(liba_SRCS:%.c=$(OBJDIR)/%.o) | $(COMMON_LIB_DIR)
	$(AR) -rcsv "$@" "$^"

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

