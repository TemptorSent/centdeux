TOP := ../../..
include ../CPU6-ISA-common.mk

.PHONY: all clean

GEN_HEADERS := CPU6-ISA_generated.h

all: $(GEN_HEADERS)

CPU6-ISA_generated.h:
	cat ../insts/ISA_insts.tsv \
		| awk ' \
			BEGIN { FS="\t"; }; \
			NF==2 { sub(/:/,"",$$0); grp=$$1; amode=$$2; }; \
		       	!/^#/ && NF>2 { gsub(/None/,"",$$0); print grp, amode, $$0; };'

clean:
	rm -f $(GEN_HEADERS)

include ../CPU6-ISA-end-common.mk
