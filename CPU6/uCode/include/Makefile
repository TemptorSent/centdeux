TOP := ../../..
include ../CPU6-uCode-common.mk

.PHONY: all ROMs clean

all: siglist.h

ROMs: ROMs/maprom_rom.h ROMs/uCode_rom.h
	make -C "./ROMs/" all

comments: comments/comments_generated.h
	make -C "./comments" all

siglist.h: cpu6_ICs.h Makefile
	printf "#pragma once\nenum SIGS {\n" > $@
	printf "\tSIG_LO=0, /* Always LO (0) */\n\tSIG_HI=1, /* Always HI (1) */\n" >> $@
	cat cpu6_ICs.h \
		| sed -ne 's/\(.*\.sig=\)\([^,]\+\)\(,.*\)/\t\2,/p' \
		| grep -v -e 'SIG_LO' -e 'SIG_HI' \
		| sort -u \
		| sed -e '$$ s/,/\n};\n/' \
		>> $@

clean:
	rm -f siglist.h

include ../CPU6-uCode-end-common.mk
