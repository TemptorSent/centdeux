#include "uI.h"
#include "cpu6_ICs.h"

void uI_decode(uI_t *uI) {
	/* Decode ALU */
	uI->ALU->rA=BITRANGE(uI->uIW,057,4); /* ALU A Address (Source) */
	uI->ALU->rB=BITRANGE(uI->uIW,053,4); /* ALU B Address (Source/Dest) */
	uI->ALU->dest=BITRANGE(uI->uIW,050,3); /* ALU I876 - Destination Control */
	uI->ALU->func=BITRANGE(uI->uIW,045,3); /* ALU I543 - Operation */
	uI->ALU->src=BITRANGE(uI->uIW,042,3); /* ALU I210 - Operand Source Select */

	/* Decode Sequencer */
	/* Decode Decoders */
	/* Decode MUXes */
	/* Decode Readers */
	/* Decode Writers */
	/* Decode System Bus */

}

