#include "uProg.h"

int main(int argc, char **argv) {
	uProg_load_uCode_rom();
	for(int i=0; i<ROMSIZE_uCode_rom; i++) {
		uI_decode(&uProg[i]);
	}
	return(0);
};
