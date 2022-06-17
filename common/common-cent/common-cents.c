#include "common-cents.h"

void cents_nstrip_0x80(char * str, uint8_t n) {
	for(int i=0; i<n; i++) {
		*(str+i) &= 0x7f;
	}
}

void cents_nadd_0x80(char * str, uint8_t n) {
	for(int i=0; i<n; i++) {
		*(str+i) |= 0x80;
	}
}


