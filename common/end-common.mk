# End portion of autodep handling
# Define SRCS with all sources to generate deps for
# Handle generating deps files for target object files automagickly
# Credit: http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/

$(DEPDIR): ; @mkdir -p $@
$(BIN): ; @mkdir -p $@
$(TOOLBIN): ; @mkdir -p $@

DEPFILES := $(SRCS:%.c=$(DEPDIR)/%.d)
$(DEPFILES):
include $(wildcard $(DEPFILES))

