#include "ginsumatic.h"
#include "CPU6-ISA.h"
#include "CPU6-ALU.h"
#include "CPU6-machine.h"
#include "CPU6-ISAemu_generated.h"

#define OPINFO_FP stderr
#define opinfo(...) fprintf(OPINFO_FP, __VA_ARGS__ )

#define CPU6_ASSERTION(...) \
	do { \
		fprintf(stderr, "CPU6_ASSERTION - %s#%d:", __FILE__, __LINE__); \
		fprintf(stderr, __VA_ARGS__); \
	} while(0)

/*
 * Bus functions
 *
*/

int CPU6_BUS_read_byte(CPU6_machine_t *m, longword_t sysaddr) {
	return(0);
}

int CPU6_BUS_write_byte(CPU6_machine_t *m, longword_t sysaddr, byte_t val) {
	return(0);
}

int CPU6_MMIO_read_byte(CPU6_machine_t *m, longword_t sysaddr) {
	return(0);
}

int CPU6_MMIO_write_byte(CPU6_machine_t *m, longword_t sysaddr, byte_t val) {
	return(0);
}

/*
 * Unknown instructions
 *
*/
static inline int CPU6_inst_unknown(CPU6_machine_t *m, byte_t op) {
	CPU6_unimplemented("Unknown operation 0x%02x",op);
	return(0);
}


/*
 * Instruction functions
 *
*/
static inline int CPU6_fetch_byte(CPU6_machine_t *m) {
	word_t pc=CPU6_get_PC(m);
	CPU6_inc_PC(m);
	return( CPU6_MEM_read_byte(m, pc) );
}

static inline int CPU6_fetch_word(CPU6_machine_t *m) {
	word_t pc=CPU6_get_PC(m);
	CPU6_inc_PC(m);
	CPU6_inc_PC(m);
	return( CPU6_MEM_read_word(m, pc) );
}

static inline int CPU6_fetch_op(CPU6_machine_t *m) {
	byte_t op=CPU6_fetch_byte(m);
	return(op);
}


/*
 * Control functions
 *
*/
static inline int CPU6_halt(CPU6_machine_t *m) {
	m->run=0;
	return(0);
}

static inline int CPU6_nop(CPU6_machine_t *m) {
	return(0);
}

static inline int CPU6_delay(CPU6_machine_t *m) {
	return(0);
}


/*
 * Flag functions
 *
*/
static inline int CPU6_set_flags(CPU6_machine_t *m, enum CPU6_FLAGS f) {
	m->flags.value |= f;
	return(0);
}

static inline int CPU6_clear_flags(CPU6_machine_t *m, enum CPU6_FLAGS f) {
	m->flags.value &= ~(f);
	return(0);
}

static inline int CPU6_toggle_flags(CPU6_machine_t *m, enum CPU6_FLAGS f) {
	m->flags.value ^= f;
	return(0);
}

static inline int CPU6_check_flags_set(CPU6_machine_t *m, enum CPU6_FLAGS f) {
	return( ((m->flags.value & f)==f)? 1 : 0 );
}

static inline int CPU6_check_flags_clear(CPU6_machine_t *m, enum CPU6_FLAGS f) {
	return( (m->flags.value & f)? 0 : 1 );
}


/*
 * Hardware state functions
 *
*/
static inline int CPU6_check_sense_switch_set(CPU6_machine_t *m, byte_t sw) {
	return( (m->sense_switches & (1<<sw))? 1 : 0 );
}

static inline int CPU6_check_tty_state(CPU6_machine_t *m, char st) {
	CPU6_unimplemented("Attempted to check teletype state - Where's the TTY?");
	return(0);
}

static inline int CPU6_check_parity_state(CPU6_machine_t *m, char st) {
	CPU6_unimplemented("Attempted to check parity state - Parity of what?");
	return(0);
}

static inline int CPU6_tty_enable_link_out(CPU6_machine_t *m) {
	CPU6_unimplemented("Attempted to use Teletype Enable Link Out (ELO) funciton - Where's the TTY?");
	return(0);
}


/*
 * Register functions
 *
*/

#define ILREG(m,r) ( (m->LV<<4) | r )

static inline int CPU6_get_reg8(CPU6_machine_t *m, nibble_t reg) {
	return( CPU6_MEM_read_byte(m, ILREG(m,reg) ) );
}

static inline int CPU6_get_reg16(CPU6_machine_t *m, nibble_t reg) {
	return( CPU6_MEM_read_word(m, ILREG(m,reg) ) );
}

static inline int CPU6_set_reg8(CPU6_machine_t *m, nibble_t reg, byte_t val) {
	CPU6_MEM_write_byte(m, ILREG(m,reg), val);
	return(0);
}

static inline int CPU6_set_reg16(CPU6_machine_t *m, nibble_t reg, word_t val) {
	CPU6_MEM_write_word(m, ILREG(m,reg), val);
	return(0);
}

static inline int CPU6_reg_push_byte(CPU6_machine_t *m, nibble_t reg, byte_t val) {
	word_t sp= CPU6_get_reg16(m, reg);
	sp--;
	CPU6_MEM_write_byte(m,sp,val);
	CPU6_set_reg16(m, reg, sp);
	return(0);
}

static inline int CPU6_reg_push_word(CPU6_machine_t *m, nibble_t reg, word_t val) {
	CPU6_reg_push_byte(m, reg, WORD_LSB(val));
	CPU6_reg_push_byte(m, reg, WORD_MSB(val));
	return(0);
}

static inline int CPU6_reg_pop_byte(CPU6_machine_t *m, nibble_t reg) {
	word_t sp= CPU6_get_reg16(m, reg);
	CPU6_set_reg16(m, reg, sp+1);
	return(CPU6_MEM_read_byte(m,sp));
}

static inline int CPU6_reg_pop_word(CPU6_machine_t *m, nibble_t reg) {
	byte_t msb= CPU6_reg_pop_byte(m, reg);
	byte_t lsb= CPU6_reg_pop_byte(m, reg);
	return(BYTES_TO_WORD(msb,lsb));
}

/*
 * Stack functions
 *
*/
static inline int CPU6_stack_push_byte(CPU6_machine_t *m, byte_t val) {
	return(CPU6_reg_push_byte(m,C6_REG_S, val));
}

static inline int CPU6_stack_push_word(CPU6_machine_t *m, word_t val) {
	return(CPU6_reg_push_word(m,C6_REG_S, val));
}

static inline int CPU6_stack_pop_byte(CPU6_machine_t *m) {
	return(CPU6_reg_pop_byte(m,C6_REG_S));
}

static inline int CPU6_stack_pop_word(CPU6_machine_t *m) {
	return(CPU6_reg_pop_word(m,C6_REG_S));
}

static inline int CPU6_stack_push_reg8(CPU6_machine_t *m, nibble_t reg) {
	return( CPU6_reg_push_byte(m,C6_REG_S, CPU6_get_reg8(m,reg)) );
}

static inline int CPU6_stack_push_reg16(CPU6_machine_t *m, nibble_t reg) {
	return( CPU6_reg_push_word(m,C6_REG_S, CPU6_get_reg16(m,reg)) );
}

static inline int CPU6_stack_pop_reg8(CPU6_machine_t *m, nibble_t reg) {
	CPU6_set_reg8(m, reg, CPU6_reg_pop_byte(m,C6_REG_S));
	return(0);
}

static inline int CPU6_stack_pop_reg16(CPU6_machine_t *m, nibble_t reg) {
	CPU6_set_reg16(m, reg, CPU6_reg_pop_word(m,C6_REG_S));
	return(0);
}

static inline int CPU6_stack_push_reg_range(CPU6_machine_t *m) {
	byte_t args=CPU6_fetch_byte(m);
	byte_t start=BYTE_MSN(args);
	byte_t range=BYTE_LSN(args);
	for(int i=range; i >= 0; i--) {
		CPU6_reg_push_byte(m,C6_REG_S, CPU6_get_reg8(m, (start + i) ) );
	}
	return(0);
}

static inline int CPU6_stack_pop_reg_range(CPU6_machine_t *m) {
	byte_t args=CPU6_fetch_byte(m);
	byte_t start=BYTE_MSN(args);
	byte_t range=BYTE_LSN(args);
	for(int i=0; i <= range ; i++) {
		CPU6_set_reg8(m, (start + i), CPU6_reg_pop_byte(m,C6_REG_S));
	}
	return(0);
}


/*
 * Interrupt functions
 *
*/
static inline int CPU6_set_IE(CPU6_machine_t *m) {
	m->IE=1;
	return(0);
}

static inline int CPU6_clear_IE(CPU6_machine_t *m) {
	m->IE=0;
	return(0);
}

static inline int CPU6_get_IE(CPU6_machine_t *m) {
	return(m->IE);
}

static inline int CPU6_get_LV(CPU6_machine_t *m) {
	return(m->LV);
}

static inline int CPU6_set_LV(CPU6_machine_t *m, byte_t lv) {
	m->LV=lv;
	return(0);
}

static inline int CPU6_return_int(CPU6_machine_t *m, char mode) {
	byte_t unk, lv, st, map;
	word_t regC,regCn,regX;
	switch(mode) {
		case '0':
			CPU6_set_reg16(m,C6_REG_P,CPU6_get_PC(m));
		case 'M':
			regC=CPU6_get_reg16(m,C6_REG_C);
			regCn= BYTES_TO_WORD(
				NIBBLES_TO_BYTE( m->flags.value, BYTE_LSN(WORD_MSB(regC)) ),
				NIBBLES_TO_BYTE( BYTE_MSN(WORD_LSB(regC)), m->MAP )
			);
			CPU6_set_reg16(m, C6_REG_C, regCn);
			CPU6_set_LV(m, BYTE_MSN(WORD_LSB(regC)) );
			break;
		case 'S':
			unk=CPU6_stack_pop_byte(m);
			regX=CPU6_stack_pop_word(m);
			lv=CPU6_stack_pop_byte(m);
			map=CPU6_stack_pop_byte(m);

			CPU6_set_reg16(m,C6_REG_X,regX);
			CPU6_set_LV(m,lv);
			CPU6_set_MAP(m,map);
			break;
	}

	return(0);
};



/*
 * PC functions
 *
*/
static inline int CPU6_get_PC(CPU6_machine_t *m) {
	return(m->PC);
}

static inline int CPU6_set_PC(CPU6_machine_t *m, word_t addr) {
	m->PC= addr;
	return(0);
}

static inline int CPU6_inc_PC(CPU6_machine_t *m) {
	m->PC++;
	return(0);
}

static inline int CPU6_xfr_PC_X(CPU6_machine_t *m) {
	CPU6_set_reg16(m, C6_REG_X, CPU6_get_PC(m));
	return(0);
}

static inline int CPU6_cond_branch(CPU6_machine_t *m, bit_t cond) {
	word_t disp= (int8_t)CPU6_fetch_byte(m);
	word_t ea= CPU6_get_PC(m) + disp;
	if(cond) { CPU6_set_PC(m,ea); }
	return(0);
}

static inline int CPU6_MEM_JMP_decode(CPU6_machine_t *m, byte_t amode) {
	word_t ea= CPU6_fetch_word_amode_mem(m, amode);
	CPU6_set_PC(m,ea);
	return(0);
}

/*
 * Subroutine functions
 *
*/
static inline int CPU6_MEM_JSR_decode(CPU6_machine_t *m, byte_t amode) {
	word_t ea= CPU6_fetch_word_amode_mem(m, amode);

	CPU6_stack_push_reg16(m, C6_REG_X);
	CPU6_set_reg16(m, C6_REG_X, CPU6_get_PC(m) );
	CPU6_set_PC(m, ea);
	CPU6_set_reg16(m, C6_REG_P, CPU6_get_PC(m) );
	return(0);
}


static inline int CPU6_return_sub(CPU6_machine_t *m) {
	CPU6_set_PC(m, CPU6_get_reg16(m, C6_REG_X) );
	CPU6_stack_pop_reg16(m,C6_REG_X);
	return(0);
}

/*
 * Memory functions
 *
*/

static inline int CPU6_MEM_read_byte(CPU6_machine_t *m, word_t addr) {
	if(addr < 0x0100) { /* Register File */
		return(m->RF[addr]);
	}

	if(addr > 0xefff) { /* MMIO Range */
		return(CPU6_MMIO_read_byte(m,addr));
	}

	return(CPU6_MEM_MAP_read_byte(m,addr));
}

static inline int CPU6_MEM_read_word(CPU6_machine_t *m, word_t addr) {
	byte_t msb= CPU6_MEM_read_byte(m, addr);
	byte_t lsb= CPU6_MEM_read_byte(m, addr+1);
	return(BYTES_TO_WORD(msb,lsb));
}

static inline int CPU6_MEM_write_byte(CPU6_machine_t *m, word_t addr, byte_t val) {
	if(addr < 0x0100) { /* Register File */
		m->RF[addr]=val;
		return(0);
	}

	if(addr > 0xefff) { /* MMIO Range */
		return(CPU6_MMIO_write_byte(m,addr,val));
	}

	CPU6_MEM_MAP_write_byte(m,addr,val);
	return(0);
}

static inline int CPU6_MEM_write_word(CPU6_machine_t *m, word_t addr, word_t val) {
	CPU6_MEM_write_byte(m, addr, WORD_MSB(val));
	CPU6_MEM_write_byte(m, addr+1, WORD_LSB(val));
	return(0);
}


static inline int CPU6_get_MAP(CPU6_machine_t *m) {
	return(m->MAP);
}

static inline int CPU6_set_MAP(CPU6_machine_t *m, octal_t map) {
	m->MAP= map;
	return(0);
}

static inline int CPU6_MEM_MAP_translate(CPU6_machine_t *m, word_t addr) {
	byte_t xa= m->MAPTABLE[ ( (addr&0xf800)>>8 ) | m->MAP ];
	return( (xa<<10) | (addr&0x07ff) );
}

static inline int CPU6_MEM_MAP_read_byte(CPU6_machine_t *m, word_t addr) {
	longword_t sysaddr=CPU6_MEM_MAP_translate(m, addr);
	return( CPU6_BUS_read_byte(m, sysaddr) );
}

static inline int CPU6_MEM_MAP_write_byte(CPU6_machine_t *m, word_t addr, byte_t val) {
	longword_t sysaddr=CPU6_MEM_MAP_translate(m, addr);
	return( CPU6_BUS_write_byte(m, sysaddr, val) );
}

static inline int CPU6_MEM_LD_decode(CPU6_machine_t *m, bit_t opw, byte_t amode, nibble_t reg) {
	if(opw) { CPU6_set_reg16(m,reg, CPU6_fetch_word_amode_mem(m, amode) ); }
	else { CPU6_set_reg8(m,reg|C6_REG_LSB, CPU6_fetch_byte_amode_mem(m, amode) ); }
	return(0);
}

static inline int CPU6_MEM_ST_decode(CPU6_machine_t *m, bit_t opw, byte_t amode, nibble_t reg) {
	if(opw) { CPU6_MEM_write_word(m, CPU6_fetch_addr_amode_mem(m, amode), CPU6_get_reg16(m,reg) ); }
	else { CPU6_MEM_write_byte(m, CPU6_fetch_addr_amode_mem(m, amode), CPU6_get_reg8(m,reg|C6_REG_LSB) ); }
	return(0);
}


static inline int CPU6_fetch_byte_amode_mem(CPU6_machine_t *m, byte_t amode) {
	return( CPU6_MEM_read_byte(m, CPU6_fetch_addr_amode_mem(m,amode) ) );
}

static inline int CPU6_fetch_word_amode_mem(CPU6_machine_t *m, byte_t amode) {
	return( CPU6_MEM_read_word(m, CPU6_fetch_addr_amode_mem(m,amode) ) );
}

static inline int CPU6_fetch_addr_amode_mem(CPU6_machine_t *m, byte_t amode) {
	switch(amode) {
		case AMODE_MEM_LIT: return(m->PC);
		case AMODE_MEM_DIR: return( CPU6_fetch_word(m) );
		case AMODE_MEM_IND: return( CPU6_MEM_read_word(m, CPU6_fetch_word(m) ) );
		case AMODE_MEM_PCREL_DIR: return( (word_t)( m->PC + (int8_t)CPU6_fetch_byte(m) ) );
		case AMODE_MEM_PCREL_IND: return( CPU6_MEM_read_word(m, (word_t)( m->PC + (int8_t)CPU6_fetch_byte(m) ) ) );
		case AMODE_MEM_IDX: return( CPU6_fetch_addr_amode_idx(m) );
		case AMODE_MEM_IMPL_IDX_A: return( CPU6_get_reg16(m,C6_REG_A) );
		case AMODE_MEM_IMPL_IDX_B: return( CPU6_get_reg16(m,C6_REG_B) );
		case AMODE_MEM_IMPL_IDX_X: return( CPU6_get_reg16(m,C6_REG_X) );
		case AMODE_MEM_IMPL_IDX_Y: return( CPU6_get_reg16(m,C6_REG_Y) );
		case AMODE_MEM_IMPL_IDX_Z: return( CPU6_get_reg16(m,C6_REG_Z) );
		case AMODE_MEM_IMPL_IDX_S: return( CPU6_get_reg16(m,C6_REG_S) );
		case AMODE_MEM_IMPL_IDX_C: return( CPU6_get_reg16(m,C6_REG_C) );
		case AMODE_MEM_IMPL_IDX_P: return( CPU6_get_reg16(m,C6_REG_P) );

	}
	return(0);
}

static inline int CPU6_fetch_addr_amode_idx(CPU6_machine_t *m) {
	ISA_amode_idx_t idx;
	word_t regval, ea;
	byte_t disp=0;
	/* Fetch our argument byte */
	idx.byte=CPU6_fetch_byte(m);
	/* If we're using a displacement mode, fetch the displacement byte */
	if(idx.displacement) { disp=CPU6_fetch_byte(m); }

	/* Read the value from our register */
	regval=CPU6_get_reg16(m,idx.reg);

	/* Predecrement if requested */
	if(idx.dec) { regval--; CPU6_set_reg16(m,idx.reg, regval); }

	/* Calculate our effective address */
	ea=regval+disp;

	/* Get indirect effective address if requested */
	if(idx.indirect) { ea=CPU6_MEM_read_word(m,ea); }

	/* Post increment if requested */
	if(idx.inc) { regval++; CPU6_set_reg16(m,idx.reg, regval); }

	return(ea);
}



/*
 * ALU functions
 *
*/
static inline int CPU6_ALU_op1r(CPU6_machine_t *m, bit_t opw, byte_t op, int reg) {
	ISA_amode_alu1_arg_t arg={};
	word_t abase=0;
	byte_t inb=0;
	word_t inw=0;

	/* Takes explicit register */
	if(reg<0) {
		arg.byte=CPU6_fetch_byte(m);
		reg=arg.reg;
		/* Word operation */
		if(opw) {
			reg=arg.regw<<1;
			/* Extended addressing modes for word operations when low bit of sr is 1 */
			if(arg.sx) {
				abase=CPU6_fetch_word(m);
				/* Register indexed when sr > 0, direct address when 0 */
				if(arg.regw) {
					inw=CPU6_MEM_read_byte(m, abase+(int16_t)CPU6_get_reg16(m,reg) );
				} else {
					inw=CPU6_MEM_read_byte(m, abase);
				}
			} else { inw=CPU6_get_reg16(m, reg); }
		} else {
		/* Byte operation */
			inb=CPU6_get_reg8(m,reg);
		}
	} else {
	/* Uses implicit Register */
		if(opw) { inw=CPU6_get_reg16(m, reg); }
		else { inb=CPU6_get_reg8(m, reg); }
	}

	if(opw) {
		CPU6_set_reg16(m, reg, CPU6_ALU_op1w_get_result(op,inw,arg.val,m->flags.L,&m->flags) );
	} else {
		CPU6_set_reg8(m, reg, CPU6_ALU_op1b_get_result(op,inb,arg.val,m->flags.L,&m->flags) );
	}

	return(0);
}

static inline int CPU6_ALU_op2r(CPU6_machine_t *m, bit_t opw, byte_t op, int sr, int dr) {
	ISA_amode_alu2_arg_t arg={};
	byte_t v1b=0, v2b=0;
	word_t v1w=0, v2w=0;
	byte_t sx=0;

	/* Takes explicit registers */
	if(sr<0 && dr<0) {
		arg.byte=CPU6_fetch_byte(m);
		sx= (arg.sx1<<1) | (arg.sx0<<0);
		sr=arg.sr;
		dr=arg.dr;
		/* Word operation */
		if(opw) {
			sr=arg.srw<<1;
			dr=arg.drw<<1;
			v2w=CPU6_get_reg16(m, sr);

			/* Extended addressing modes for word operations when low bit of sr and/or dr is 1 */

			switch( sx ) {
				case AMODE_ALU2W_SRC_REG: v1w=CPU6_get_reg16(m, sr); break;
				case AMODE_ALU2W_SRC_DIR: v1w=CPU6_MEM_read_word(m,CPU6_fetch_word(m)); break;
				case AMODE_ALU2W_SRC_LIT: v1w=CPU6_fetch_word(m);
				case AMODE_ALU2W_SRC_IDX_DISP:
					v1w=CPU6_MEM_read_word(m, (CPU6_get_reg16(m,sr) + CPU6_fetch_word(m)) );
					break;
			}
		} else {
		/* Byte operation */
			v1w=CPU6_get_reg8(m,sr);
			v2w=CPU6_get_reg8(m,dr);
		}

	/* Uses implicit Registers */
	} else if(sr > -1 && dr > -1) {
		if(opw) { v1w=CPU6_get_reg16(m, sr); v2w=CPU6_get_reg16(m, dr); }
		else { v1b=CPU6_get_reg8(m, sr); v2b=CPU6_get_reg8(m, dr); }

	/* We don't have a mode that uses one implicit and one explict, throw a fit */
	} else {
		CPU6_unimplemented("CPU6_ALU_op2r: Invalid argument - sr and dr must both either be explicit or implicit.");
		return(-1);
	}


	if(opw) {
		CPU6_set_reg16(m, dr, CPU6_ALU_op2w_get_result(op,v1w,v2w,m->flags.L,&m->flags) );
	} else {
		CPU6_set_reg8(m, dr, CPU6_ALU_op2b_get_result(op,v1b,v2b,m->flags.L,&m->flags) );
	}

	return(0);
}

inline static int CPU6_ext_inst_MAP(CPU6_machine_t *m) {

}

inline static int CPU6_ext_inst_DMA_ISR(CPU6_machine_t *m) {

}
inline static int CPU6_ext_inst_bignum(CPU6_machine_t *m) {

}
inline static int CPU6_ext_inst_memop(CPU6_machine_t *m) {

}



/***************
 * Main switch *
 ***************/
int CPU6_eval_op(CPU6_machine_t *m, byte_t op) {
	byte_t opg=op&0xf0;
	byte_t opsub=op&0x0f;
	bit_t opw=opg&0x1;

	if(
		/* Special instructions at 0x6 and 0x7 offsets for all groups after 0x30 group */
		(opg > 0x3 && ((opsub&0xe) == 0x6) )
		/* 0x2e/0x2f and 0x7e/0x7f special instructions */
		|| ( (opg == 0x2 || opg == 0x7) && ((opsub&0xe) == 0xe) )
		/* Oddball 0x78 special instructions */
		|| (op == 0x78)
	) {
		opinfo("Ext.");
		switch(op) {
			/* 2e instructions for MAP manipulation */
			case 0x2e:
				opinfo("MAP: 0x2e");
				return( CPU6_ext_inst_MAP(m) );
			/* 2f instructions for DMA and internal status register control */
			case 0x2f:
				opinfo("IRQ_ISR: 0x2f");
				return( CPU6_ext_inst_DMA_ISR(m) );
			case 0x46: opinfo("BIGNUM: 0x46"); return( CPU6_ext_inst_bignum(m) );
			case 0x47: opinfo("MEMOP: 0x47"); return( CPU6_ext_inst_memop(m) );
			case 0x56:
			case 0x57:
			case 0x66:
			case 0x67:
			case 0x76:
			case 0x77:
			case 0x78:
			case 0x7e:
			case 0x7f:
			case 0x86:
			case 0x87:
			case 0x96:
			case 0x97:
			case 0xa6:
			case 0xa7:
			case 0xb6:
			case 0xb7:
			case 0xc6:
			case 0xc7:
			case 0xd6:
			case 0xd7:
			case 0xe6:
			case 0xe7:
			case 0xf6:
			case 0xf7:
				CPU6_inst_unknown(m,op);
				return(-1);
			default:
				CPU6_ASSERTION("Special instruction switch fell through!");
				return(-1);
		}

	}

	switch(opg) {	
		case OPG_CTRL:
			opinfo("CTRL: 0x%0x",op);
			switch(opsub) {
				case 0x0: return( CPU6_halt(m) );
				case 0x1: return( CPU6_nop(m) );
				case 0x2: return( CPU6_set_flags(m,C6_FLAG_F) );
				case 0x3: return( CPU6_clear_flags(m,C6_FLAG_F) );
				case 0x4: return( CPU6_set_IE(m) );
				case 0x5: return( CPU6_clear_IE(m) );
				case 0x6: return( CPU6_set_flags(m,C6_FLAG_L) );
				case 0x7: return( CPU6_clear_flags(m,C6_FLAG_L) );
				case 0x8: return( CPU6_toggle_flags(m,C6_FLAG_F) );

				case 0x9: return( CPU6_return_sub(m) );
				case 0xa: return( CPU6_return_int(m,0) );
				case 0xb: return( CPU6_return_int(m,'M') );
				case 0xc: return( CPU6_tty_enable_link_out(m) );
				case 0xd: return( CPU6_xfr_PC_X(m) );
				case 0xe: return( CPU6_delay(m) );
				case 0xf: return( CPU6_return_int(m,'S') );
			}
			break;
		case OPG_COND:
			opinfo("COND: 0x%0x",op);
			switch(opsub) {
				case 0x0: return( CPU6_cond_branch(m, CPU6_check_flags_set(m,C6_FLAG_L) ) );
				case 0x1: return( CPU6_cond_branch(m, CPU6_check_flags_clear(m,C6_FLAG_L) ) );
				case 0x2: return( CPU6_cond_branch(m, CPU6_check_flags_set(m,C6_FLAG_F) ) );
				case 0x3: return( CPU6_cond_branch(m, CPU6_check_flags_clear(m,C6_FLAG_F) ) );
				case 0x4: return( CPU6_cond_branch(m, CPU6_check_flags_set(m,C6_FLAG_V) ) );
				case 0x5: return( CPU6_cond_branch(m, CPU6_check_flags_clear(m,C6_FLAG_V) ) );
				case 0x6: return( CPU6_cond_branch(m, CPU6_check_flags_set(m,C6_FLAG_M) ) );
				case 0x7: return( CPU6_cond_branch(m, CPU6_check_flags_clear(m,C6_FLAG_M) ) );
				case 0x8: return( CPU6_cond_branch(m, CPU6_check_flags_clear(m, (C6_FLAG_M | C6_FLAG_V)) ) );
				case 0x9: return( CPU6_cond_branch(m, CPU6_check_flags_set(m, (C6_FLAG_M | C6_FLAG_V)) ) );
				case 0xa: return( CPU6_cond_branch(m, CPU6_check_sense_switch_set(m,1) ) );
				case 0xb: return( CPU6_cond_branch(m, CPU6_check_sense_switch_set(m,2) ) );
				case 0xc: return( CPU6_cond_branch(m, CPU6_check_sense_switch_set(m,3) ) );
				case 0xd: return( CPU6_cond_branch(m, CPU6_check_sense_switch_set(m,4) ) );
				case 0xe: return( CPU6_cond_branch(m, CPU6_check_tty_state(m,'M') ) );
				case 0xf: return( CPU6_cond_branch(m, CPU6_check_parity_state(m,'E') ) );
			}
			break;
		case OPG_ALU1B:
			/* These should be caught by special case handling before main switch */
			if( (opsub & 0xe) == 0xe ) {
				CPU6_ASSERTION("Special instruction op 0x%02x not caught before main switch", op);
			}
		case OPG_ALU1W:
			opinfo("ALU1: 0x%0x",op);
			switch(opsub) {
				case 0x0: return( CPU6_ALU_op1r(m,opw,C6_ALU_OP_INC,-1) );
				case 0x1: return( CPU6_ALU_op1r(m,opw,C6_ALU_OP_DEC,-1) );
				case 0x2: return( CPU6_ALU_op1r(m,opw,C6_ALU_OP_CLR,-1) );
				case 0x3: return( CPU6_ALU_op1r(m,opw,C6_ALU_OP_INV,-1) );
				case 0x4: return( CPU6_ALU_op1r(m,opw,C6_ALU_OP_ASR,-1) );
				case 0x5: return( CPU6_ALU_op1r(m,opw,C6_ALU_OP_SL,-1) );
				case 0x6: return( CPU6_ALU_op1r(m,opw,C6_ALU_OP_RRC,-1) );
				case 0x7: return( CPU6_ALU_op1r(m,opw,C6_ALU_OP_RLC,-1) );
				case 0x8: return( CPU6_ALU_op1r(m,opw,C6_ALU_OP_INC,C6_REG_A) );
				case 0x9: return( CPU6_ALU_op1r(m,opw,C6_ALU_OP_DEC,C6_REG_A) );
				case 0xa: return( CPU6_ALU_op1r(m,opw,C6_ALU_OP_CLR,C6_REG_A) );
				case 0xb: return( CPU6_ALU_op1r(m,opw,C6_ALU_OP_INV,C6_REG_A) );
				case 0xc: return( CPU6_ALU_op1r(m,opw,C6_ALU_OP_ASR,C6_REG_A) );
				case 0xd: return( CPU6_ALU_op1r(m,opw,C6_ALU_OP_SL,C6_REG_A) );
				case 0xe: return( CPU6_ALU_op1r(m,1,C6_ALU_OP_INC,C6_REG_X) );
				case 0xf: return( CPU6_ALU_op1r(m,1,C6_ALU_OP_DEC,C6_REG_X) );
			}
			break;
		case OPG_ALU2B:
		case OPG_ALU2W:

			opinfo("ALU2: 0x%0x",op);
			switch(opsub) {
				case 0x0: return( CPU6_ALU_op2r(m,opw,C6_ALU_OP_ADD,-1,-1) );
				case 0x1: return( CPU6_ALU_op2r(m,opw,C6_ALU_OP_SUB,-1,-1) );
				case 0x2: return( CPU6_ALU_op2r(m,opw,C6_ALU_OP_AND,-1,-1) );
				case 0x3: return( CPU6_ALU_op2r(m,opw,C6_ALU_OP_OR,-1,-1) );
				case 0x4: return( CPU6_ALU_op2r(m,opw,C6_ALU_OP_XOR,-1,-1) );
				case 0x5: return( CPU6_ALU_op2r(m,opw,C6_ALU_OP_XFR,-1,-1) );
				case 0x6:
				case 0x7: 
					CPU6_ASSERTION("Special instruction op 0x%02x not caught before main switch", op);
					return(-1);
				case 0x8: return( CPU6_ALU_op2r(m,opw,C6_ALU_OP_ADD,C6_REG_A,C6_REG_B) );
				case 0x9: return( CPU6_ALU_op2r(m,opw,C6_ALU_OP_SUB,C6_REG_A,C6_REG_B) );
				case 0xa: return( CPU6_ALU_op2r(m,opw,C6_ALU_OP_AND,C6_REG_A,C6_REG_B) );
				case 0xb: return( CPU6_ALU_op2r(m,opw,C6_ALU_OP_XFR,C6_REG_A,C6_REG_X) );
				case 0xc: return( CPU6_ALU_op2r(m,opw,C6_ALU_OP_XFR,C6_REG_A,C6_REG_Y) );
				case 0xd: return( CPU6_ALU_op2r(m,opw,C6_ALU_OP_XFR,C6_REG_A,C6_REG_B) );
				case 0xe: return( CPU6_ALU_op2r(m,opw,C6_ALU_OP_XFR,C6_REG_A,C6_REG_Z) );
				case 0xf: return( CPU6_ALU_op2r(m,opw,C6_ALU_OP_XFR,C6_REG_A,C6_REG_S) );
			}
			break;
		case OPG_MEMX:
			if(opsub < 0x6 ) {
				opinfo("LDX: 0x%0x",op);
				return( CPU6_MEM_LD_decode(m,opw,opsub,C6_REG_X) );
			} else if ( (opsub&0x8) && opsub < (0x8+0x6) ) {
				opinfo("STX: 0x%0x",op);
				return( CPU6_MEM_ST_decode(m,opw,(opsub-0x8),C6_REG_X) );
			} else {
				CPU6_ASSERTION("Special instruction op 0x%02x not caught before main switch", op);
				return(-1);
			}
			break;
		case OPG_FLOW:
			switch(opsub) {
				case 0x0:
				case 0x1:
				case 0x2:
				case 0x3:
				case 0x4:
				case 0x5:
					opinfo("JMP: 0x%0x",op);
					return( CPU6_MEM_JMP_decode(m,opsub) );
				case 0x6:
				case 0x7: 
				case 0x8:
					CPU6_ASSERTION("Special instruction op 0x%02x not caught before main switch", op);
					return(-1);
				case 0x9:
				case 0xa:
				case 0xb:
				case 0xc:
				case 0xd:
					opinfo("JSR: 0x%0x",op);
					return( CPU6_MEM_JSR_decode(m,(opsub-0x8)) );
				case 0xe:
					opinfo("PUSHRR: 0x%0x",op);
					return( CPU6_stack_push_reg_range(m) );
				case 0xf:
					opinfo("POPRR: 0x%0x",op);
					return( CPU6_stack_pop_reg_range(m) );
			}
			break;
		case OPG_LDAB:
		case OPG_LDAW:
		case OPG_STAB:
		case OPG_STAW:
		case OPG_LDBB:
		case OPG_LDBW:
		case OPG_STBB:
		case OPG_STBW:
			/* Special instrustions should be handled in logic before main switch */
			if ( (opsub & 0xe) == 0x6 ) {
				CPU6_ASSERTION("Special instruction op 0x%02x not caught before main switch", op);
				return(-1);
			}

			if(opg&0x2) {
				opinfo("ST: 0x%0x",op);
				return( CPU6_MEM_ST_decode(m,opw,opsub,(opg&0x4)?C6_REG_B:C6_REG_A) );
			} else {
				opinfo("LD: 0x%0x",op);
				return( CPU6_MEM_LD_decode(m,opw,opsub,(opg&0x4)?C6_REG_B:C6_REG_A) );
			}
			break;
	}
	return(0);
};


int CPU6_run_next_instruction(CPU6_machine_t *m) {
	byte_t op;
	int ret;
	op = CPU6_fetch_op(m);
	ret = CPU6_eval_op(m,op);
	return(ret);
}

