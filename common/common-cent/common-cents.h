#pragma once
#include <stdint.h>
#include <limits.h>

#define CENT_HELLO "Hellorld!"

void cents_ncopy_strip_0x80(char *d, char *s, size_t n);
void cents_nstrip_0x80(char *str, size_t n);
void cents_ncopy_add_0x80(char *d, char *s, size_t n);
void cents_nadd_0x80(char *str, size_t n);

