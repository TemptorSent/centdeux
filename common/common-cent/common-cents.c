#include "common-cents.h"

#define cents_char_mask_and_or(c, and_mask, or_mask) (((c) & (and_mask)) | (or_mask) )

static inline void cents_ncopy_mask_and_or(char *d, char *s, char and_mask, char or_mask, size_t n) {
	for(int i=0; i<n; i++) {
		*(d+i)= cents_char_mask_and_or( *(s+i), and_mask, or_mask);
	}
}


void cents_nstrip_0x80(char *str, size_t n) {
	cents_ncopy_mask_and_or(str,str,0x7f,0x00,n);
}

void cents_ncopy_strip_0x80(char *d, char *s, size_t n) {
	cents_ncopy_mask_and_or(d,s,0x7f,0x00,n);
}


void cents_ncopy_add_0x80(char *d, char *s, size_t n) {
	cents_ncopy_mask_and_or(d,s,0x00,0x80,n);
}

void cents_nadd_0x80(char *str, size_t n) {
	cents_ncopy_mask_and_or(str,str,0x00,0x80,n);
}
