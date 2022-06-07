#pragma once
#include <stdio.h>
#include "types-common.h"

#define CPU6_unimplemented(...) do { printf("CPU6 unimplemented: "); printf(__VA_ARGS__); printf("\n"); } while(0)

enum CPU6_OP_GROUPS {
	OPG_CTRL=0x0,
	OPG_COND=0x1,
	OPG_ALU1B=0x2,
	OPG_ALU1W=0x3,
	OPG_ALU2B=0x4,
	OPG_ALU2W=0x5,
	OPG_MEMX=0x6,
	OPG_FLOW=0x7,
	OPG_LDAB=0x8,
	OPG_LDAW=0x9,
	OPG_STAB=0xa,
	OPG_STAW=0xb,
	OPG_LDBB=0xc,
	OPG_LDBW=0xd,
	OPG_STBB=0xe,
	OPG_STBW=0xf
};

enum CPU6_AMODE_GROUPS {
	AMODEG_NONE=0x0,
	AMODEG_DISP,
	AMODEG_ALU1,
	AMODEG_ALU2,
	AMODEG_ALU2W,
	AMODEG_MEM,
	AMODEG_EXT
};

enum CPU6_AMODES_ALU2W {
	AMODE_ALU2W_SR_DR=0x0,
	AMODE_ALU2W_SR_DIR=0x1,
	AMODE_ALU2W_SR_LIT=0x2,
	AMODE_ALU2W_SR_IDX=0x3
};


enum CPU6_AMODES_MEM {
	AMODE_MEM_LIT=0x0,
	AMODE_MEM_DIR=0x1,
	AMODE_MEM_IND=0x2,
	AMODE_MEM_DIR_DISP=0x3,
	AMODE_MEM_IND_DISP=0x4,
	AMODE_MEM_IDX=0x5,
	AMODE_MEM_IMPL_IDX_A=0x8,
	AMODE_MEM_IMPL_IDX_B=0x9,
	AMODE_MEM_IMPL_IDX_X=0xa,
	AMODE_MEM_IMPL_IDX_Y=0xb,
	AMODE_MEM_IMPL_IDX_Z=0xc,
	AMODE_MEM_IMPL_IDX_S=0xd,
	AMODE_MEM_IMPL_IDX_C=0xe,
	AMODE_MEM_IMPL_IDX_P=0xf
};

enum CPU6_AMODES_IDX {
	AMODE_IDX_DIR=0x0,
	AMODE_IDX_DIR_INC=0x1,
	AMODE_IDX_DIR_DEC=0x2,
	AMODE_IDX_IND=0x4,
	AMODE_IDX_IND_INC=0x5,
	AMODE_IDX_IND_DEC=0x6,
	AMODE_IDX_DIR_DISP=0x8,
	AMODE_IDX_DIR_DISP_INC=0x9,
	AMODE_IDX_DIR_DISP_DEC=0xa,
	AMODE_IDX_IND_DISP=0xc,
	AMODE_IDX_IND_DISP_INC=0xd,
	AMODE_IDX_IND_DISP_DEC=0xe
};

enum CPU6_FLAGS {
	C6_FLAG_F=0x1<<0,
	C6_FLAG_L=0x1<<1,
	C6_FLAG_M=0x1<<2,
	C6_FLAG_V=0x1<<3,
};

enum CPU6_REGS {
	C6_REG_A=0x0,
	C6_REG_MSB=0x0,
	C6_REG_A0=0x0,
	C6_REG_LSB=0x1,
	C6_REG_A1=0x1,
	C6_REG_B=0x2,
	C6_REG_B0=0x2,
	C6_REG_B1=0x3,
	C6_REG_X=0x4,
	C6_REG_X0=0x4,
	C6_REG_X1=0x5,
	C6_REG_Y=0x6,
	C6_REG_Y0=0x6,
	C6_REG_Y1=0x7,
	C6_REG_Z=0x8,
	C6_REG_Z0=0x8,
	C6_REG_Z1=0x9,
	C6_REG_S=0xa,
	C6_REG_S0=0xa,
	C6_REG_S1=0xb,
	C6_REG_C=0xc,
	C6_REG_C0=0xc,
	C6_REG_C1=0xd,
	C6_REG_P=0xe,
	C6_REG_P0=0xe,
	C6_REG_P1=0xf
};

enum CPU6_CONTEXT_BITS {
	C6_CONTEXT_MAP_b0=0x0,
	C6_CONTEXT_MAP_b1=0x1,
	C6_CONTEXT_MAP_b2=0x2,
	C6_CONTEXT_MAP_b3=0x3,
	C6_CONTEXT_FLAG_L=0x4,
	C6_CONTEXT_FLAG_F=0x5,
	C6_CONTEXT_FLAG_M=0x6,
	C6_CONTEXT_FLAG_V=0x7,
	C6_CONTEXT_IPL_b0=0xc,
	C6_CONTEXT_IPL_b1=0xd,
	C6_CONTEXT_IPL_b2=0xe,
	C6_CONTEXT_IPL_b3=0xf
};

typedef union CPU6_context_reg_t {
	word_t	word;
	struct {
		word_t map:4;
		word_t flag_l:1;
		word_t flag_f:1;
		word_t flag_m:1;
		word_t flag_v:1;
		word_t unused:4;
		word_t ipl:4;
	};
} CPU6_context_reg_t;


typedef const struct ISA_inst_t {
	enum CPU6_AMODES_MEM amode_mem;
	byte_t	opcode;
	byte_t	range;
	char *mnemonic;
	enum CPU6_AMODES_IDX amode_idx;
} ISA_inst_t;

typedef const struct ISA_inst_group_t {
	char *mn_root;
	enum CPU6_OP_GROUPS opg;
	enum CPU6_AMODE_GROUPS addrg;
	ISA_inst_t *insts[];

} ISA_inst_group_t;

typedef const union ISA_amode_idx_t {
	byte_t	byte;
	struct {
		byte_t inc:1;
		byte_t dec:1;
		byte_t indirect:1;
		byte_t displacement:1;
		byte_t reg:4;
	};
} ISA_amode_idx_t;

typedef union ISA_amode_alu2w_t {
	byte_t byte;
	struct {
		byte_t sr:3;
		byte_t sx1:1;
		byte_t dr:3;
		byte_t sx0:1;
	};
} ISA_amode_alu2w_t;
