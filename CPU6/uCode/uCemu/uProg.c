#include "ROMs/uCode_rom.h"
#include "uProg.h"

uI_t uProg[ROMSIZE_uCode_rom];

void uProg_load_uCode_rom() {
	for(int i=0; i<ROMSIZE_uCode_rom; i++) {
		uProg[i].uIW=uCode_rom[i];
		uProg[i].uADDR=i;
	}
}
