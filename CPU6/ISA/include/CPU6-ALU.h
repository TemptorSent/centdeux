#pragma once
#include "types-common.h"

#define CPU6_ALU_op1b_get_result CPU6_ALU_C_op_byte
#define CPU6_ALU_op1w_get_result CPU6_ALU_C_op_word
#define CPU6_ALU_op2b_get_result CPU6_ALU_C_op_byte
#define CPU6_ALU_op2w_get_result CPU6_ALU_C_op_word

typedef union CPU6_ALU_flags_t {
	nibble_t value;
	nibble_t nibble;
	struct {
		nibble_t F:1;
		nibble_t L:1;
		nibble_t M:1;
		nibble_t V:1;
		nibble_t  :4;
	};
	struct {
		nibble_t OVF:1;
		nibble_t Cout:1;
		nibble_t FN:1;
		nibble_t FZ:1;
		nibble_t  :4;
	};
} CPU6_ALU_flags_t;

enum CPU6_ALU_OPS_UNARY {
	C6_ALU_OP_INC,
	C6_ALU_OP_DEC,
	C6_ALU_OP_CLR,
	C6_ALU_OP_INV,
	C6_ALU_OP_ASR,
	C6_ALU_OP_SL,
	C6_ALU_OP_RRC,
	C6_ALU_OP_RLC,
	C6_ALU_OP_ADD,
	C6_ALU_OP_SUB,
	C6_ALU_OP_AND,
	C6_ALU_OP_OR,
	C6_ALU_OP_XOR,
	C6_ALU_OP_XFR
};

byte_t CPU6_ALU_C_op_byte(nibble_t op, byte_t v1, byte_t v2, bit_t Cin, CPU6_ALU_flags_t *flags);
word_t CPU6_ALU_C_op_word(nibble_t op, word_t v1, word_t v2, bit_t Cin, CPU6_ALU_flags_t *flags);
