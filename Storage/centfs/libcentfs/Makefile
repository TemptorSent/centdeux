TOP := ../../..
include ../centfs-common.mk



liba_TARGET = $(CENTFS_LIB_DIR)/libcentfs$(LIBA_EXT)

liba_SRCS := centfs.c

INCS := centfs.h
centfs_INCS := $(addprefix $(CENTFS_INC_DIR)/, $(INCS))

test_PROGS := test_centfs
test_SRCS := $(addsuffix .c, $(test_PROGS))
test_EXES := $(addsuffix $(EXE_EXT), $(test_PROGS))

SRCS = $(liba_SRCS) $(test_SRCS)

.PHONY: all clean test distclean


all: $(liba_TARGET) $(centfs_INCS)


$(liba_TARGET): $(liba_SRCS:%.c=$(OBJDIR)/%.o) | $(CENTFS_LIB_DIR)
	$(AR) -rcsv "$@" "$^"

$(CENTFS_INC_DIR)/%.h: %.h | $(CENTFS_INC_DIR)
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
	rm -f $(patsubst %, "%", $(centfs_INCS))


include ../centfs-end-common.mk

