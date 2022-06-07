CC = gcc
CFLAGS = -Wall -Wno-unused -ggdb

.PHONY: all

TOP := ../../../..

include ../../CMD-uCode-common.mk

uC_ROMDIR = ../../ROMs
uC_uIWFILES = \
    CMDB311.rom \
    CMDC311.rom \
    CMDDC311.rom \
    CMDD311.rom \
    CMDED311.rom \
    CMDE311.rom \
    CMDF311.rom \
    CMDHF311.rom

uC_uIWROMS = $(addprefix $(uC_ROMDIR)/, $(uC_uIWFILES))

AUX_ROMDIR = $(uC_ROMDIR)/AUX
CMD_AUX_F808_ROM_FILE = CMDA7-F808.rom
#CMD_AUX_1123_ROM_FILE = CMDA4b-1123-6.1.rom
#CMD_AUX_1123W_ROM_FILE = CMDB3b-1123W.rom
#CMD_AUX_1123SEL_ROM_FILE = CMDJ7b-1123SEL.rom

CMD_AUX_F808_ROM = $(addprefix $(AUX_ROMDIR)/, $(CMD_AUX_F808_ROM_FILE))
#CMD_AUX_1123_ROM = $(addprefix $(AUX_ROMDIR)/, $(CMD_AUX_1123_ROM_FILE))
#CMD_AUX_1123W_ROM = $(addprefix $(AUX_ROMDIR)/, $(CMD_AUX_1123W_ROM_FILE))
#CMD_AUX_1123SEL_ROM = $(addprefix $(AUX_ROMDIR)/, $(CMD_AUX_1123SEL_ROM_FILE))

INC_GENERATE = \
    CMD_uCode_rom.h \
    CMD_AUX_F808_rom.h
#    CMD_AUX_1123_rom.h \
#    CMD_AUX_1123W_rom.h \
#    CMD_AUX_1123SEL_rom.h

all: $(INC_GENERATE)

$(TOOLBIN)/rom2include$(EXE_EXT):
	make -C $(TOP)/tools/ROM-tools rom2include

$(TOOLBIN)/romcat$(EXE_EXT):
	make -C $(TOP)/tools/ROM-tools romcat

CMD_AUX_F808_rom.h: $(CMD_AUX_F808_ROM) $(TOOLBIN)/rom2include$(EXE_EXT)
	$(TOOLBIN)/rom2include$(EXE_EXT) --name CMD_AUX_F808_rom $< > $@

#CMD_AUX_1123_rom.h: $(CMD_AUX_1123_ROM) $(TOOLBIN)/rom2include$(EXE_EXT)
#	$(TOOLBIN)/rom2include$(EXE_EXT) --name CMD_AUX_1123_rom $< > $@
#
#CMD_AUX_1123W_rom.h: $(CMD_AUX_1123_ROM) $(TOOLBIN)/rom2include$(EXE_EXT)
#	$(TOOLBIN)/rom2include$(EXE_EXT) --name CMD_AUX_1123W_rom $< > $@
#
#CMD_AUX_1123SEL_rom.h: $(CMD_AUX_1123_ROM) $(TOOLBIN)/rom2include$(EXE_EXT)
#	$(TOOLBIN)/rom2include$(EXE_EXT) --name CMD_AUX_1123SEL_rom $< > $@

CMD_uCode_rom.h: $(TOOLBIN)/romcat$(EXE_EXT) $(TOOLBIN)/rom2include$(EXE_EXT) $(uC_uIWROMS)
	$(TOOLBIN)/romcat$(EXE_EXT) --stride=8 --format=b $(uC_uIWROMS) \
		| $(TOOLBIN)/rom2include$(EXE_EXT) --word-size=8 --name=CMD_uCode_rom - \
		> $@

clean:
	rm -f $(INC_GENERATE)
