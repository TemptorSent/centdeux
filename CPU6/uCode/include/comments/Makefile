TOP := ../../../..
include ../../CPU6-uCode-common.mk

GEN_SCRIPT := $(CPU6_uCODE_TOP)/Comments/generate-comments.sh
GEN_HEADERS := comments_generated.h

.PHONY: all
	
all: $(GEN_HEADERS)
	@printf "Generating %s\n" $^


comments_generated.h: $(GEN_SCRIPT)
	$(SHELL) $(GEN_SCRIPT) > comments_generated.h

clean:
	rm -f $(GEN_HEADERS)

include ../../CPU6-uCode-end-common.mk

