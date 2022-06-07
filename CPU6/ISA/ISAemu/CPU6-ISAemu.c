#include "ginsumatic.h"
#include "CPU6-ISA.h"
#include "CPU6-ALU.h"
#include "CPU6-machine.h"
#include "CPU6-ISAemu_generated.h"


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
	m->flags |= f;
	return(0);
}

static inline int CPU6_clear_flags(CPU6_machine_t *m, enum CPU6_FLAGS f) {
	m->flags &= ~(f);
	return(0);
}

static inline int CPU6_toggle_flags(CPU6_machine_t *m, enum CPU6_FLAGS f) {
	m->flags ^= f;
	return(0);
}

static inline int CPU6_check_flags_set(CPU6_machine_t *m, enum CPU6_FLAGS f) {
	return( ((m->flags & f)==f)? 1 : 0 );
}

static inline int CPU6_check_flags_clear(CPU6_machine_t *m, enum CPU6_FLAGS f) {
	return( (m->flags & f)? 0 : 1 );
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
				NIBBLES_TO_BYTE( m->flags, BYTE_LSN(WORD_MSB(regC)) ),
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

	return(0);
}

static inline int CPU6_fetch_word_amode_mem(CPU6_machine_t *m, byte_t amode) {
	return(0);
}

static inline int CPU6_fetch_addr_amode_mem(CPU6_machine_t *m, byte_t amode) {
	return(0);
}


static inline int CPU6_inst_2e(CPU6_machine_t *m) {
	return(0);
}

static inline int CPU6_inst_2f(CPU6_machine_t *m) {
	return(0);
}


/*
 * ALU functions
 *
*/
static inline int CPU6_ALU_op1r(CPU6_machine_t *m, bit_t opw, byte_t op, int dr) {
	return(0);
}

static inline int CPU6_ALU_op2r(CPU6_machine_t *m, bit_t opw, byte_t op, int sr, int dr) {
	return(0);
}



/***************
 * Main switch *
 ***************/
int CPU6_eval_op(CPU6_machine_t *m, byte_t op) {
	byte_t opg=op&0xf0;
	byte_t opsub=op&0x0f;
	bit_t opw=opg&0x1;

	switch(opg) {	
		case OPG_CTRL:
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
			switch(opsub) {
				case 0xe: return( CPU6_inst_2e(m) );
				case 0xf: return( CPU6_inst_2f(m) );
			}
		case OPG_ALU1W:
			switch(opsub) {
				case 0x0: return( CPU6_ALU_op1r(m,opw,C6_ALU_OP_INC,-1) );
				case 0x1: return( CPU6_ALU_op1r(m,opw,C6_ALU_OP_DEC,-1) );
				case 0x2: return( CPU6_ALU_op1r(m,opw,C6_ALU_OP_CLR,-1) );
				case 0x3: return( CPU6_ALU_op1r(m,opw,C6_ALU_OP_INV,-1) );
				case 0x4: return( CPU6_ALU_op1r(m,opw,C6_ALU_OP_ASR,-1) );
				case 0x5: return( CPU6_ALU_op1r(m,opw,C6_ALU_OP_LSL,-1) );
				case 0x6: return( CPU6_ALU_op1r(m,opw,C6_ALU_OP_ROR,-1) );
				case 0x7: return( CPU6_ALU_op1r(m,opw,C6_ALU_OP_ROL,-1) );
				case 0x8: return( CPU6_ALU_op1r(m,opw,C6_ALU_OP_INC,C6_REG_A) );
				case 0x9: return( CPU6_ALU_op1r(m,opw,C6_ALU_OP_DEC,C6_REG_A) );
				case 0xa: return( CPU6_ALU_op1r(m,opw,C6_ALU_OP_CLR,C6_REG_A) );
				case 0xb: return( CPU6_ALU_op1r(m,opw,C6_ALU_OP_INV,C6_REG_A) );
				case 0xc: return( CPU6_ALU_op1r(m,opw,C6_ALU_OP_ASR,C6_REG_A) );
				case 0xd: return( CPU6_ALU_op1r(m,opw,C6_ALU_OP_LSL,C6_REG_A) );
				case 0xe: return( CPU6_ALU_op1r(m,1,C6_ALU_OP_INC,C6_REG_X) );
				case 0xf: return( CPU6_ALU_op1r(m,1,C6_ALU_OP_DEC,C6_REG_X) );
			}
			break;
		case OPG_ALU2B:
		case OPG_ALU2W:
			switch(opsub) {
				case 0x0: return( CPU6_ALU_op2r(m,opw,C6_ALU_OP_ADD,-1,-1) );
				case 0x1: return( CPU6_ALU_op2r(m,opw,C6_ALU_OP_SUB,-1,-1) );
				case 0x2: return( CPU6_ALU_op2r(m,opw,C6_ALU_OP_AND,-1,-1) );
				case 0x3: return( CPU6_ALU_op2r(m,opw,C6_ALU_OP_OR,-1,-1) );
				case 0x4: return( CPU6_ALU_op2r(m,opw,C6_ALU_OP_XOR,-1,-1) );
				case 0x5: return( CPU6_ALU_op2r(m,opw,C6_ALU_OP_XFR,-1,-1) );
				case 0x6:
				case 0x7: return( CPU6_inst_unknown(m,op) );
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
			if(opsub < 0x6 ) { return( CPU6_MEM_LD_decode(m,opw,opsub,C6_REG_X) ); }
			else if ( (opsub&0x8) && opsub < (0x8+0x6) ) { return( CPU6_MEM_ST_decode(m,opw,(opsub-0x8),C6_REG_X) ); }
			else { return( CPU6_inst_unknown(m,op) ); }
			break;
		case OPG_FLOW:
			switch(opsub) {
				case 0x0:
				case 0x1:
				case 0x2:
				case 0x3:
				case 0x4:
				case 0x5:
					return( CPU6_MEM_JMP_decode(m,opsub) );
				case 0x6:
				case 0x7:
				case 0x8:
					return( CPU6_inst_unknown(m,op) );
				case 0x9:
				case 0xa:
				case 0xb:
				case 0xc:
				case 0xd:
					return( CPU6_MEM_JSR_decode(m,(opsub-0x8)) );
				case 0xe: return( CPU6_stack_push_reg_range(m) );
				case 0xf: return( CPU6_stack_pop_reg_range(m) );
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
			if(opg&0x2) {
				return( CPU6_MEM_ST_decode(m,opw,opsub,(opg&0x4)?C6_REG_B:C6_REG_A) );
			} else {
				return( CPU6_MEM_LD_decode(m,opw,opsub,(opg&0x4)?C6_REG_B:C6_REG_A) );
			}
			break;
	}
	return(0);
};


int main(int argc, char **argv) {

	return(0);
};
