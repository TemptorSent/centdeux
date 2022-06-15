#include <stdio.h>
#include "types-common.h"
#include "ALU_am2901_core.h"

static inline nibble_t am2901_read_Rmux(am2901_core_t *core) {
	switch(core->internal.Rmux) {
		case 0: case 'Z': case '0': return(0);
		case 1: case 'D': return( core->in->D & 0xf );
		case 2: case 'A': return( core->internal.A & 0xf );
	}
	return(0);
}

static inline nibble_t am2901_read_Smux(am2901_core_t *core) {
	switch(core->internal.Smux) {
		case 0: case 'Z': case '0': return(0);
		case 1: case 'A': return(core->internal.A);
		case 2: case 'B': return(core->internal.B);
		case 3: case 'Q': return(core->internal.Q);
	}
	return(0);
}

static inline nibble_t am2901_read_RAMmux(am2901_core_t *core) {
	switch(core->RAM_shift->mux) {
		case 0: case 'X': return(0);
		case 1: case 'D': return( ( (core->internal.F>>1) | ((*(core->RAM_shift->msb)&0x1)<<3) ) & 0xf ); 
		case 2: case 'N': return( (core->internal.F) & 0xf );
		case 3: case 'U': return( ( (core->internal.F<<1) | (*(core->RAM_shift->lsb)&0x1) ) & 0xf );
	}
	return(0);
}

static inline nibble_t am2901_read_Qmux(am2901_core_t *core) {
	switch(core->Q_shift->mux) {
		case 0: case 'X': return(0);
		case 1: case 'D': return( ( (core->internal.Q>>1) | ((*(core->Q_shift->msb)&0x1)<<3) ) & 0xf); 
		case 2: case 'N': return( (core->internal.F) & 0xf );
		case 3: case 'U': return( ( (core->internal.Q<<1) | (*(core->Q_shift->lsb)&0x1) ) & 0xf);
	}
	return(0);
}

static inline nibble_t am2901_readYmux(am2901_core_t *core) {
	return( (core->internal.Ymux=='A'?core->internal.A:core->internal.F) & 0xf);
}


static inline char *am2901_source_operand_decode(am2901_core_t *core) {
	octal_t src=core->inst->src;
	/* Set MUX connections for R (0,A,D) and S (0,A,B,Q) operands */
	core->internal.Rmux=am2901_source_operands[src][0];
	core->internal.Smux=am2901_source_operands[src][1];
	return(am2901_source_operand_mnemonics[src]);
}

static inline char *am2901_destination_decode(am2901_core_t *core) {
	octal_t dest=core->inst->dest;
	/* Set output source ('A' or 'F') */
	core->internal.Ymux=am2901_destinations[dest][0];

	/* Set RAM enable, mux, and RAM0/RAM3 directions */
	core->internal.RAM_WE=am2901_destinations[dest][1];
	core->RAM_shift->mux=am2901_destinations[dest][2];
	core->RAM_shift->lsb_dir=am2901_destinations[dest][3];
	core->RAM_shift->msb_dir=am2901_destinations[dest][4];

	/* Set Q enable, mux, and RAM0/RAM3 directions */
	core->internal.Q_WE=am2901_destinations[dest][5];
	core->Q_shift->mux=am2901_destinations[dest][6];
	core->Q_shift->lsb_dir=am2901_destinations[dest][7];
	core->Q_shift->msb_dir=am2901_destinations[dest][8];

	/* Return the mnemonic */
	return(am2901_destination_mnemonics[dest]);
}

nibble_t am2901_function_decode(am2901_core_t *core) {
	octal_t func= core->inst->func&0x7;
	nibble_t R= am2901_read_Rmux(core)&0xf;
	nibble_t S= am2901_read_Smux(core)&0xf;

	nibble_t F;
	bit_t Cn,Co;
	bit_t P,P_,P3,P2,P1,P0,Pall;
	bit_t G,G_,G3,G2,G1,G0,Gany;
	bit_t C4,C3,C2,C1,C0;
	bit_t OVR;

	Cn=core->in->Cin&0x1;

	/* Complement R or S before calculating P and G as required */
	switch(func) {
		case SUBR: case NOTRS: case EXOR: R = (~R)&0xf; break;
		case SUBS: S = (~S)&0xf;
	}

	P=(R|S)&0xf;
	G=(R&S)&0xf;

	P0=(P&0x1)?1:0;
	P1=(P&0x2)?1:0;
	P2=(P&0x4)?1:0;
	P3=(P&0x8)?1:0;
	Pall=(P==0xf)?1:0;	// = P3&P2&P1&P0

	G0=(G&0x1)?1:0;
	G1=(G&0x2)?1:0;
	G2=(G&0x4)?1:0;
	G3=(G&0x8)?1:0;
	Gany=(G)?1:0;		// = G3|G2|G1|G0

	C0= Cn;
	C1= G0 | ( P0 & C0 );	// = G0|(P0&Cn)
	C2= G1 | ( P1 & C1 );	// = G1|(P1&G0)|(P1&P0&Cn)
	C3= G2 | ( P2 & C2 );	// = G2|(P2&G1)|(P2&P1&G0)|(P2&P1&P0&Cn)
	C4= G3 | ( P3 & C3 );	// = G3|(P3&G2)|(P3&P2&G1)|(P3&P2&P1&G0)|(P3&P2&P1&P0&Cn)


	/* Cn+1= Gn | (Pn & Cn )
	 * #define ISBITSET(v,b) ( ( (v) & (1<<(b)) )? 1 : 0 )
	 * for(b=0; b<size; b++) { C |=( ( ISBITSET(G,b) | ( ISBITSET(P,b) & ISBITSET(C,b) ) ) << (b+1)
	 * */

	switch(func) {
		case ADD:
		case SUBR:
		case SUBS:
			F=( R + S + Cn ) & 0xf;
			P_=(~Pall) & 0x1;
			G_=(~((G3)|(P3&G2)|(P3&P2&G1)|(P3&P2&P1&G0))) & 0x1;
			Co=C4;
			OVR=C3^C4;
			break;
		case OR:
			F=( R | S ) & 0xf;
			P_=0;
			G_=Pall;
			Co=( (~Pall) | Cn) & 0x1;
			OVR=Co;
			break;
		case AND:
		case NOTRS:
			F=( R & S ) & 0xf;
			P_=0;
			G_= (~Gany) & 0x1;
			Co= Gany | Cn;
			OVR=Co;
			break;
		case EXOR:
		case EXNOR:
			F=( ( R & S ) | ( ((~R)&0xf) & ((~S)&0xf) ) ) & 0xf;
			P_=Gany;
			G_= G3 | (P3&G2) | (P3&P2&G1) | Pall;
			Co= (~( G_ & ( G0 | (~Cn&0x1) ) )) & 0x1;
			OVR=( ( ~P2 | (~G2&~P1) | (~G2&~G1&~P0) | (~G2&~G1&~G0&Cn) )
			    ^ ( ~P3 | (~G3&~P2) | (~G3&~G2&~P1) | (~G3&~G2&~G1&~P0) | (~G3&~G2&~G1&~G0&Cn) )
			    ) & 0x1;
			/*
			 *
			 *    P= ~P
			 *    G= ~G
			 *    OVR=( ( P2 | (G2&P1) | (G2&G1&P0) | (G2&G1&G0&Cn) )
			 *        ^ ( P3 | (G3&P2) | (G3&G2&P1) | (G3&G2&G1&P0) | (G3&G2&G1&G0&Cn) )
			 *    ( (
			 */
			break;
	}
	core->internal.F=F;

	core->out->lookahead->P_=P_;
	core->out->lookahead->G_=G_;

	core->out->flags->Cout=Co;
	core->out->flags->OVR=OVR;
	core->out->flags->F3=(F&0x8)?1:0;
	core->out->flags->FZ=F?0:1;

	return(F);
}



char *am2901_latch_AB(am2901_core_t *core) {
	/* Latch A & B from selected RAM addresses */
	core->internal.A=core->internal.RAM[core->inst->rA];
	core->internal.B=core->internal.RAM[core->inst->rB];
	return(0);
}


char *am2901_update_RAM(am2901_core_t *core) {
	/* Update RAM at address in ADDR_B if RAM_WE is high */
	if(core->internal.RAM_WE) { core->internal.RAM[core->inst->rB]=am2901_read_RAMmux(core); }
	return(0);
}

char *am2901_update_Q(am2901_core_t *core) {
	/* Update Q regsister if Q_WE is high */
	if(core->internal.Q_WE) { core->internal.Q=am2901_read_Qmux(core); }
	return(0);

}

char *am2901_update_shifter(am2901_core_t *core) {
	/* Update shifter outputs */
	if(core->Q_shift->lsb_dir=='O') { core->Q_shift->lsb=(core->internal.Q>>0)&0x1; }
	if(core->Q_shift->msb_dir=='O') { core->Q_shift->msb=(core->internal.Q>>3)&0x1; }
	if(core->RAM_shift->lsb_dir=='O') { core->RAM_shift->lsb=(core->internal.F>>0)&0x1; }
	if(core->RAM_shift->msb_dir=='O') { core->RAM_shift->msb=(core->internal.F>>3)&0x1; }
}

char *am2901_update_outputs(am2901_core_t *core) {
	/* Zero flag - must be "pulled high" externally, this acts as an open-collector output*/
	if(core->internal.F&0xf){*(core->out->flags->FZ=0;}

	/* F3 flag - set if MSB of F is 1 (sign bit if MSB of word)*/
	*(core->out->flags->F3)=core->internal.F&0x8?1:0;

	/* Update Y outputs if enabled */
	if(!*(core->in->OE_)){*(core->out->Y)=am2901_readYmux(core);}
	return(0);
}

int am2901_shifter_init(am2901_shifter_t *shift, bit_t *msb, bit_t *lsb) {

	shift->mux='N';

	shift->msb_dir='Z';
	shift->msb=msb;
	*(shift->msb)=0;

	shift->lsb_dir='Z';
	shift->lsb=lsb;
	*(shift->lsb)=0;
}

int am2901_core_init(am2901_core_t *core, char *id ) {
	/* Set the identifier for this core */
	core->id=id;

	/* Clear internal state */
	core->internal.F=0;

	/* Zero A & B registers */
	core->internal.A=0;
	core->internal.B=0;

	/* Zero Q register and clear its enable */
	core->internal.Q=0;
	core->internal.Q_WE=0;

	/* Initilize Rmux & Smux */
	core->internal.Rmux='A';
	core->internal.Smux='A';

	/* Initilize Ymux */
	core->internal.Ymux='F';

	/* Zero RAM and clear its enable */
	core->internal.RAM_WE=0;
	for(int r=0; r<16; r++) {
		core->internal.RAM[r]=0;
	}

	return(0);
}




