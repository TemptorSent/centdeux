#pragma once
#include "types-common.h"

typedef struct CPU6_machine_t {
	bit_t run;
	word_t PC;
	int flags;
	byte_t RF[0x100];
	byte_t MAPTABLE[0x100];
	octal_t MAP;
	bit_t IE;
	nibble_t LV;
	byte_t sense_switches;
		

} CPU6_machine_t;
