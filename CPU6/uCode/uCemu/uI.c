#include "uI.h"
#include "cpu6_ICs.h"
#include "ginsumatic.h"

void uI_decode(uI_t *uI) {
	/* Decode ALU */
	uI->ALU_inst.rA=BITRANGE(uI->uIW,057,4); /* ALU A Address (Source) */
	uI->ALU_inst.rB=BITRANGE(uI->uIW,053,4); /* ALU B Address (Source/Dest) */
	uI->ALU_inst.dest=BITRANGE(uI->uIW,050,3); /* ALU I876 - Destination Control */
	uI->ALU_inst.func=BITRANGE(uI->uIW,045,3); /* ALU I543 - Operation */
	uI->ALU_inst.src=BITRANGE(uI->uIW,042,3); /* ALU I210 - Operand Source Select */

	/* Decode Sequencer */
	/* Decode Decoders */
	/* Decode MUXes */
	/* Decode Readers */
	/* Decode Writers */
	/* Decode System Bus */

}

