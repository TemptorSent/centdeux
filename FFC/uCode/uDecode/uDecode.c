
#include "ginsumatic.h"
#include "ROMs/FFC_uCode_rom.h"

int main(int argc, char **argv) {
	uint64_t uIW;
	for(int i=0; i < ROMSIZE_FFC_uCode_rom; i++) {
		uIW=FFC_uCode_rom[i];
		printf("0x%03x: 0x%016lx  0x%016lx\n",i, uIW,~uIW);
	}
	return(0);
}
