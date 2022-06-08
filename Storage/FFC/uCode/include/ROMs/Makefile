CC = gcc
CFLAGS = -Wall -Wno-unused -ggdb

.PHONY: all

TOP := ../../../../..

include ../../FFC-uCode-common.mk

uC_ROMDIR = ../../ROMs
uC_uIWFILES = \
    FFCH1-1.6.rom \
    FFCH2-1.6.rom \
    FFCH3-1.6.rom \
    FFCH4-1.8.rom \
    FFCH5-1.8.rom \
    FFCH6-1.6.rom \
    FFCH7-1.8.rom \
    FFCH8-1.6.rom

uC_uIWROMS = $(addprefix $(uC_ROMDIR)/, $(uC_uIWFILES))

AUX_ROMDIR = $(uC_ROMDIR)/aux_roms
FFC_AUX_F800_ROM_FILE = FFCA3-F800.rom
FFC_AUX_1123_ROM_FILE = FFCA4b-1123-6.1.rom
FFC_AUX_1123W_ROM_FILE = FFCB3b-1123W.rom
FFC_AUX_1123SEL_ROM_FILE = FFCJ7b-1123SEL.rom

FFC_AUX_F800_ROM = $(addprefix $(AUX_ROMDIR)/, $(FFC_AUX_F800_ROM_FILE))
FFC_AUX_1123_ROM = $(addprefix $(AUX_ROMDIR)/, $(FFC_AUX_1123_ROM_FILE))
FFC_AUX_1123W_ROM = $(addprefix $(AUX_ROMDIR)/, $(FFC_AUX_1123W_ROM_FILE))
FFC_AUX_1123SEL_ROM = $(addprefix $(AUX_ROMDIR)/, $(FFC_AUX_1123SEL_ROM_FILE))

INC_GENERATE = \
    FFC_uCode_rom.h \
    FFC_AUX_F800_rom.h \
    FFC_AUX_1123_rom.h \
    FFC_AUX_1123W_rom.h \
    FFC_AUX_1123SEL_rom.h

all: $(INC_GENERATE)

$(TOOLBIN)/rom2include$(EXE_EXT):
	make -C $(TOP)/tools/ROM-tools rom2include

$(TOOLBIN)/romcat$(EXE_EXT):
	make -C $(TOP)/tools/ROM-tools romcat

FFC_AUX_F800_rom.h: $(FFC_AUX_F800_ROM) $(TOOLBIN)/rom2include$(EXE_EXT)
	$(TOOLBIN)/rom2include$(EXE_EXT) --name FFC_AUX_F800_rom $< > $@

FFC_AUX_1123_rom.h: $(FFC_AUX_1123_ROM) $(TOOLBIN)/rom2include$(EXE_EXT)
	$(TOOLBIN)/rom2include$(EXE_EXT) --name FFC_AUX_1123_rom $< > $@

FFC_AUX_1123W_rom.h: $(FFC_AUX_1123_ROM) $(TOOLBIN)/rom2include$(EXE_EXT)
	$(TOOLBIN)/rom2include$(EXE_EXT) --name FFC_AUX_1123W_rom $< > $@

FFC_AUX_1123SEL_rom.h: $(FFC_AUX_1123_ROM) $(TOOLBIN)/rom2include$(EXE_EXT)
	$(TOOLBIN)/rom2include$(EXE_EXT) --name FFC_AUX_1123SEL_rom $< > $@

FFC_uCode_rom.h: $(TOOLBIN)/romcat$(EXE_EXT) $(TOOLBIN)/rom2include$(EXE_EXT) $(uC_uIWROMS)
	$(TOOLBIN)/romcat$(EXE_EXT) --stride=8 --format=b $(uC_uIWROMS) \
		| $(TOOLBIN)/rom2include$(EXE_EXT) --word-size=8 --name=FFC_uCode_rom - \
		> $@

clean:
	rm -f $(INC_GENERATE)
