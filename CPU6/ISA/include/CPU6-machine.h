#pragma once
#include "types-common.h"
#include "CPU6-ALU.h"

typedef struct CPU6_machine_t {
	bit_t run;
	word_t PC;
	CPU6_ALU_flags_t flags;
	byte_t RF[0x100];
	byte_t MAPTABLE[0x100];
	octal_t MAP;
	bit_t IE;
	nibble_t LV;
	byte_t sense_switches;
		

} CPU6_machine_t;

int CPU6_eval_op(CPU6_machine_t *m, byte_t op);
