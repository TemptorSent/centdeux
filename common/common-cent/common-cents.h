#pragma once
#include <stdint.h>

#define CENT_HELLO "Hellorld!"

void cents_nstrip_0x80(char * str, uint8_t n);
void cents_nadd_0x80(char * str, uint8_t n);

