#include "common-cents.h"


void cents_nstrip_0x80(char *str, size_t n) {
	for(size_t i=0; i<n; i++) {
		*(str+i)=UNCENT_CHAR((uint8_t)(*(str+i)));
	}
}

void cents_ncopy_strip_0x80(char *d, char *s, size_t n) {
	for(size_t i=0; i<n; i++) {
		*(d+i)=UNCENT_CHAR((uint8_t)(*(s+i)));
	}
}


void cents_nadd_0x80(char *str, size_t n) {
	for(size_t i=0; i<n; i++) {
		*(str+i)=CENT_CHAR((uint8_t)(*(str+i)));
	}
}

void cents_ncopy_add_0x80(char *d, char *s, size_t n) {
	for(size_t i=0; i<n; i++) {
		*(d+i)=CENT_CHAR((uint8_t)(*(s+i)));
	}
}


