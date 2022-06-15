#include <stdio.h>
#include "../common/logic-common.h"
#include "am2901.h"

char *am2901_clock_state_setup_H(am2901_device_t *dev){
	/* Decode destination portion of instruction */
	am2901_destination_decode(dev);
	return(0);
}

char *am2901_clock_state_edge_HL(am2901_device_t *dev) {
	/* Latch A & B from RAM address on A0-A3 */
	dev->A=dev->RAM[*(dev->inst->rA)];
	dev->B=dev->RAM[*(dev->inst->rB)];
	return(0);
}


char *am2901_clock_state_setup_L(am2901_device_t *dev){
	/* Decode source operands portion of instruction */
	am2901_source_operand_decode(dev);

	/* Decode ALU operation */
	am2901_function_decode(dev);

	/* Update RAM at address in ADDR_B if RAM_EN is high */
	if(dev->RAM_EN) { dev->RAM[*(dev->inst->rB)]=am2901_read_RAMmux(dev); }
	return(0);
}

char *am2901_clock_state_edge_LH(am2901_device_t *dev) {
	/* Update Q regsister if Q_EN is high */
	if(dev->Q_EN) { dev->Q=am2901_read_Qmux(dev); }
	return(0);

}

char *am2901_update(am2901_device_t *dev) {
	//printf("clk=%s\n",CLOCK_STATE_NAME_FULL(*(dev->clk)));
	switch(*(dev->clk)) {
		case CLK_HL: am2901_clock_state_edge_HL(dev); break;
		case CLK_LO: am2901_clock_state_setup_L(dev); break;
		case CLK_LH: am2901_clock_state_edge_LH(dev); break;
		case CLK_HI: am2901_clock_state_setup_H(dev); break;
		default:
			printf("Unknown clock state: %0x", S_(clk)); break;
	}

	/* Update shifter outputs */
	if(dev->Q_shift->lsb_dir=='O') { *(dev->Q_shift->lsb)=(dev->Q>>0)&0x1; }
	if(dev->Q_shift->msb_dir=='O') { *(dev->Q_shift->msb)=(dev->Q>>3)&0x1; }
	if(dev->RAM_shift->lsb_dir=='O') { *(dev->RAM_shift->lsb)=(dev->F>>0)&0x1; }
	if(dev->RAM_shift->msb_dir=='O') { *(dev->RAM_shift->msb)=(dev->F>>3)&0x1; }

	/* Zero flag - must be "pulled high" externally, this acts as an open-collector output*/
	if(dev->F&0xf){*(dev->output->FZ)=0;}

	/* F3 flag - set if MSB of F is 1 (sign bit if MSB of word)*/
	*(dev->output->F3)=dev->F&0x8?1:0;

	/* Update Y outputs if enabled */
	if(!*(dev->input->OE_)){*(dev->output->Y)=am2901_readYmux(dev);}
	return(0);
}

int am2901_device_init(am2901_device_t *dev, char* id,
	clock_state_t *clk, /* Clock state from clockline */
	am2901_inst_t *inst,

	/* Instruction word of 9 bits in 3 groups of 3 bit (octal) values */
	octal_t *I210, /* Source operand octal value */
	octal_t *I543, /* ALU function octal value */ 
	octal_t *I876, /* Destination octal value */

	/* RAM input shifter */
	bit_t *RAM0, /* LSB in RAM data input shift line */
	bit_t *RAM3, /* MSB in RAM data input shift line */
	nibble_t *ADDR_A, /* Address in RAM to use for "A" (read) */
	nibble_t *ADDR_B, /* Address in RAM to use for "B" (read/write) */
	/* External data input */
	nibble_t *D, /* Direct data input */

	/* ALU */
	bit_t *Cn, /* Carry-in */
	bit_t *P_, /* Propagate signal */
	bit_t *G_, /* Generate signal */
	bit_t *Co, /* Carry-out */
	bit_t *OVR, /* Overflow flag */

	/* Q input shifter */
	bit_t *Q0, /* LSB in Q shift line */
	bit_t *Q3, /* MSB in Q shift line */
	/* Outputs */
	bit_t *FZ, /* Zero flag output */
	bit_t *F3, /* High bit set (Negitive) flag output */
	nibble_t *Y, /* Output value */
	bit_t *OE_ /* Output Enable (Active LOW) HiZ=1 */

	) {
	dev->clk=clk;
	dev->id=id;

	/* Instruction word */
	dev->I210=(enum am2901_source_operand_code *)I210;
	dev->I543=(enum am2901_function_code *)I543;
	dev->I876=(enum am2901_destination_code *)I876;

	/* RAM input shifter */
	defS_(RAM0);
	defS_(RAM3);
	defS_(ADDR_A);
	defS_(ADDR_B);

	/* External data input */
	defS_(D);

	/* ALU */
	defS_(Cn);
	defS_(P_);
	defS_(G_);
	defS_(Co);
	defS_(OVR);

	/* Q input shifter */
	defS_(Q0);
	defS_(Q3);

	/* Outputs */
	defS_(FZ);
	defS_(F3);
	defS_(Y);
	defS_(OE_);

	/* Clear internal state */
	I_(F)=0;
	/* Zero A & B registers */
	I_(A)=0;
	I_(B)=0;
	/* Zero Q register and clear it's input enable */
	I_(Q)=0;
	I_(Q_EN)=0;
	/* Initilize Rmux & Smux */
	I_(Rmux)='A';
	I_(Smux)='A';
	/* Initilize RAM shifter */
	I_(RAM0_DIR)='Z';
	I_(RAM3_DIR)='Z';
	I_(RAMmux)='N';
	/* Initilize Q Shifter */
	I_(Q0_DIR)='Z';
	I_(Q3_DIR)='Z';
	I_(Qmux)='N';
	/* Initilize Ymux */
	I_(Ymux)='F';

	for(int r=0; r<16; r++) {
		dev->RAM[r]=0;
	}
	return(0);
}



