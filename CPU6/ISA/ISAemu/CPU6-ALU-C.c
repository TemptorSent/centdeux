#include "types-common.h"
#include "ginsumatic.h"
#include "CPU6-ISA.h"
#include "CPU6-ALU.h"



#define _CPU6_CHECK_OVF(size,res,Cout) ( (((res) & (0x1<<(size-1)))?1:0) ^ Cout )

#define _CPU6_ALU_ADDC(size,v1,v2,Cin,res,Cout,OVF) \
	res= (v1) + (v2) + (Cin); \
	Cout= (res<(v1))? 1 : 0; \
	OVF= ISBITSETMSB( ((v1)^res) & ((v2)^res) )

#define _CPU6_ALU_AND(size,v1,v2,res) res= (v1) & (v2)

#define _CPU6_ALU_OR(size,v1,v2,res) res= v1 | v2

#define _CPU6_ALU_XOR(size,v1,v2,res) res= v1 ^ v2


#define _CPU6_SHIFT_ASR(size,val,num,res,Cout,OVF) \
	res= ((val)>>(num)) | (((val)&(0x1<<(size-1)))? ~((val)&0x0)<<(size-(num)) : 0x0 ); \
	Cout= ISBITSETLSB((val)>>(num-1)); \
	OVF= _CPU6_CHECK_OVF(size,res,Cout)

#define _CPU6_SHIFT_LSR(size,val,num,res,Cout,OVF) \
       	res= ((val)>>(num)); \
	Cout= ISBITSETLSB((val)>>(num-1)); \
	OVF= _CPU6_CHECK_OVF(size,res,Cout)

#define _CPU6_SHIFT_SL(size,val,num,res,Cout,OVF) \
	res= ((val)<<(num)); \
	Cout= ISBITSETMSB((val)<<(num-1)); \
	OVF= _CPU6_CHECK_OVF(size,res,Cout)

#define _CPU6_SHIFT_RRC(size,val,num,Cin,res,Cout,OVF) \
	res= ((val)>>(num)) | ( (((val)<<0x1)|Cin) <<(size-(num))); \
	Cout= ISBITSETLSB((val)>>(num-1)); \
	OVF= _CPU6_CHECK_OVF(size,res,Cout)

#define _CPU6_SHIFT_RLC(size,val,num,Cin,res,Cout,OVF) \
	res= ((((val)<<0x1)|Cin)<<(num)) | ((val) >>(size-(num))); \
	Cout= ISBITSETMSB((val)<<(num-1)); \
	OVF= _CPU6_CHECK_OVF(size,res,Cout)


byte_t CPU6_ALU_C_op_byte(nibble_t op, byte_t v1, byte_t v2, bit_t Cin, CPU6_ALU_flags_t *flags) {
	byte_t res=0,tmp=0;
	switch(op) {
		case C6_ALU_OP_INC: _CPU6_ALU_ADDC(8,v1,v2,0,res,tmp,flags->F); break;
		case C6_ALU_OP_DEC: _CPU6_ALU_ADDC(8,v1,~v2,0,res,tmp,flags->F); break;
		case C6_ALU_OP_CLR: _CPU6_ALU_ADDC(8,0,v2,0,res,flags->L,flags->F); break;
		case C6_ALU_OP_INV: _CPU6_ALU_ADDC(8,~v1,v2,0,res,flags->L,flags->F); break;
		case C6_ALU_OP_ASR: _CPU6_SHIFT_ASR(8,v1,1+v2,res,flags->L,flags->F); break;
		case C6_ALU_OP_SL:  _CPU6_SHIFT_SL(8,v1,1+v2,res,flags->L,flags->F); break;
		case C6_ALU_OP_RRC: _CPU6_SHIFT_RRC(8,v1,1+v2,Cin,res,flags->L,flags->F); break;
		case C6_ALU_OP_RLC: _CPU6_SHIFT_RLC(8,v1,1+v2,Cin,res,flags->L,flags->F); break;
		case C6_ALU_OP_ADD: _CPU6_ALU_ADDC(8,v1,v2,0,res,flags->L,flags->F); break;
		case C6_ALU_OP_SUB: _CPU6_ALU_ADDC(8,v1,~v2,1,res,flags->L,flags->F); break;
		case C6_ALU_OP_AND: _CPU6_ALU_AND(8,v1,v2,res); break;
		case C6_ALU_OP_OR:  _CPU6_ALU_OR(8,v1,v2,res); break;
		case C6_ALU_OP_XOR: _CPU6_ALU_XOR(8,v1,v2,res); break;
		case C6_ALU_OP_XFR: _CPU6_ALU_OR(8,v1,0,res); break;
	}

	/* Set M (sign) and V (zero) flags based on result */
	flags->M=(res&(0x1<<8))?1:0;
	flags->V=(res)?0:1;

	return(res);


}

word_t CPU6_ALU_C_op_word(nibble_t op, word_t v1, word_t v2, bit_t Cin, CPU6_ALU_flags_t *flags) {
	word_t res=0,tmp=0;
	switch(op) {
		case C6_ALU_OP_INC: _CPU6_ALU_ADDC(16,v1,v2,0,res,tmp,flags->F); break;
		case C6_ALU_OP_DEC: _CPU6_ALU_ADDC(16,v1,~v2,0,res,tmp,flags->F); break;
		case C6_ALU_OP_CLR: _CPU6_ALU_ADDC(16,0,v2,0,res,flags->L,flags->F); break;
		case C6_ALU_OP_INV: _CPU6_ALU_ADDC(16,~v1,v2,0,res,flags->L,flags->F); break;
		case C6_ALU_OP_ASR: _CPU6_SHIFT_ASR(16,v1,1+v2,res,flags->L,flags->F); break;
		case C6_ALU_OP_SL:  _CPU6_SHIFT_SL(16,v1,1+v2,res,flags->L,flags->F); break;
		case C6_ALU_OP_RRC: _CPU6_SHIFT_RRC(16,v1,1+v2,Cin,res,flags->L,flags->F); break;
		case C6_ALU_OP_RLC: _CPU6_SHIFT_RLC(16,v1,1+v2,Cin,res,flags->L,flags->F); break;
		case C6_ALU_OP_ADD: _CPU6_ALU_ADDC(16,v1,v2,0,res,flags->L,flags->F); break;
		case C6_ALU_OP_SUB: _CPU6_ALU_ADDC(16,v1,~v2,1,res,flags->L,flags->F); break;
		case C6_ALU_OP_AND: _CPU6_ALU_AND(16,v1,v2,res); break;
		case C6_ALU_OP_OR:  _CPU6_ALU_OR(16,v1,v2,res); break;
		case C6_ALU_OP_XOR: _CPU6_ALU_XOR(16,v1,v2,res); break;
		case C6_ALU_OP_XFR: _CPU6_ALU_OR(16,v1,0,res); break;
	}

	/* Set M (sign) and V (zero) flags based on result */
	flags->M=(res&(0x1<<16))?1:0;
	flags->V=(res)?0:1;

	return(res);

}
