CC = gcc
CFLAGS = -Wall -Wno-unused -ggdb

.PHONY: all

TOP := ../../../..

include ../../CPU6-uCode-common.mk

uC_ROMDIR = $(TOP)/CPU6/uCode/ROMs
uC_uIWFILES = \
    CPU_UH3_MWK3.11_A3.11.rom \
    CPU_UL3_MWF3.11_B3.11.rom \
    CPU_UK3_MWH3.11_C3.11.rom \
    CPU_UF3_MWL3.11_D3.11.rom \
    CPU_UE3_MWM3.11_E3.11.rom \
    CPU_UJ3_MWJ3.11_F3.11.rom \
    CPU_UM3_MWE3.11_M3.11.rom

uC_uIWROMS = $(addprefix $(uC_ROMDIR)/, $(uC_uIWFILES))

uC_MAPROMFILE = CPU_UB13_MAPROM_6309.rom
uC_MAPROM = $(addprefix $(uC_ROMDIR)/, $(uC_MAPROMFILE))

INC_GENERATE = uCode_rom.h maprom_rom.h

all: $(INC_GENERATE)

$(TOOLBIN)/rom2include$(EXE_EXT):
	make -C $(TOP)/tools/ROM-tools rom2include

$(TOOLBIN)/romcat$(EXE_EXT):
	make -C $(TOP)/tools/ROM-tools romcat

maprom_rom.h: $(TOOLBIN)/rom2include$(EXE_EXT) $(uC_MAPROM)
	$(TOOLBIN)/rom2include$(EXE_EXT) --name maprom $(uC_MAPROM) > maprom_rom.h

uCode_rom.h: $(TOOLBIN)/romcat$(EXE_EXT) $(TOOLBIN)/rom2include$(EXE_EXT) $(uC_uIWROMS)
	$(TOOLBIN)/romcat$(EXE_EXT) --stride=7 --format=b $(uC_uIWROMS) \
		| $(TOOLBIN)/rom2include$(EXE_EXT) --word-size=7 --name=uCode_rom - \
		> uCode_rom.h

clean:
	rm -f $(INC_GENERATE)
