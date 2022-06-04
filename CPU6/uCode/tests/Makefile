TOP := ../../..
include ../CPU6-uCode-common.mk

.PHONY: all
.PHONY: clean
.PHONY: includes

all: $(DEPDIR) test_ICs

SRCS = test_ICs.c

includes:
	$(MAKE) -C ../include

test_ICs: test_ICs.o

#test_ICs.o: test_ICs.c ../include/cpu6_ICs.h

../include/cpu6_ICs.h: includes

clean:
	rm -f *.o *.exe *.core *.stackdump test_IC

include ../CPU6-uCode-end-common.mk
