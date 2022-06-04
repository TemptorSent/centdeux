
enum OP_GROUPS {
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

enum ADDR_GROUPS {
	ADDRG_NONE=0x0,
	ADDRG_DISP,
	ADDRG_ALU1,
	ADDRG_ALU2,
	ADDRG_MEM,
	ADDRG_EXT
};

enum ADDR_MODES_MEM {
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

typedef const struct ISA_inst_t {
	enum ADDR_MODES_MEM addrm_mem;
	byte_t	opcode;
	byte_t	range;
	char *mnemonic;
	enum ADDR_MODES_REG addrm_reg;
} ISA_inst_t;

typedef const struct ISA_inst_group_t {
	char *mn_root;
	enum OP_GROUPS opg;
	enum ADDR_GROUPS addrg;
	ISA_inst_t *insts[];

} ISA_inst_group_t;

enum ADDR_MODES_IDX {
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

enum ADDR_MODES_ALU2W {
	AMODE_ALU2W_SR_DR=0x0,
	AMODE_ALU2W_SR_DIR=0x1,
	AMODE_ALU2W_SR_LIT=0x2,
	AMODE_ALU2W_SR_IDX=0x3
};

typedef union ISA_amode_alu2w_t {
	byte_t byte;
	struct {
		byte_t sr:3;
		byte_t sx1:1;
		byte_t dr:3;
		byte_t sx0:1;
	};
} ISA_amode_alu2w_t;
