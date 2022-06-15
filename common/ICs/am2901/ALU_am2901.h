#pragma once
#include <stdint.h>
#include "types-common.h"
#include "../common/clockline.h"

enum am2901_source_operand_code { AQ=00, AB=01, ZQ=02, ZB=03, ZA=04, DA=05, DQ=06, DZ=07 };
static char am2901_source_operands[8][2] = { {'A','Q'},{'A','B'},{'0','Q'},{'0','B'},{'0','A'},{'D','A'},{'D','Q'},{'D','0'}};
static char *am2901_source_operand_mnemonics[8] = { "AQ", "AB", "ZQ", "ZB", "ZA", "DA", "DQ", "DZ" };

enum am2901_function_code { ADD=00, SUBR=01, SUBS=02, OR=03, AND=04, NOTRS=05, EXOR=06, EXNOR=07 };
static char *am2901_function_mnemonics[8]= {"ADD","SUBR","SUBS","OR","AND","NOTRS","EXOR","EXNOR"};
static char *am2901_function_symbol[8] = {"R+S+C","S-R-1+C","R-S-1+C","R|S","R&S","(~R)&S","R^S","~(R^S)"};
static char *am2901_function_symbol_complement[8] = {"R+S+C","(~R)+S+C","R+(~S)+C","R|S","R&S","(~R)&S","~((~R)^S)","~(R^S)"};

static char *am2901_function_symbol_reduced[2][3][8] = {{
	{"R+S+1","S-R","R-S","R|S","R&S","(~R)&S","R^S","~(R^S)"},
	{"S+1","S","-S","S","0","S","S","~S"},
	{"R+1","-R","R","R","0","0","R","~R"}
}, {
	{"R+S","S-R-1","R-S-1","R|S","R&S","(~R)&S","R^S","~(R^S)"},
	{"S","S-1","-S-1","S","0","S","S","~S"},
	{"R","R-1","-R-1","R","0","0","R","~R"}
}};

enum am2901_destination_code { QREG=00, NOP=01, RAMA=02, RAMF=03, RAMQD=04, RAMD=05, RAMQU=06, RAMU=07};
static char *am2901_destination_mnemonics[8] = {"QREG","NOP", "RAMA","RAMF","RAMQD","RAMD","RAMQU","RAMU"};

enum am2901_destination_fields { Ymux, RAM_EN, RAMmux, RAM0_DIR, RAM3_DIR, Q_EN, Qmux, Q0_DIR, Q3_DIR };
static char am2901_destinations[8][9] = {
	{'F', 0,'N','Z','Z', 1,'N','Z','Z'}, /* QREG, F->Q, Y=F */
	{'F', 0,'N','Z','Z', 0,'N','Z','Z'}, /* NOP Y=F */
	{'A', 1,'N','Z','Z', 0,'N','Z','Z'}, /* RAMA, F->B, Y=A */
	{'F', 1,'N','Z','Z', 0,'N','Z','Z'}, /* RAMF, F->B, Y=F */
	{'F', 1,'D','O','I', 1,'D','O','I'}, /* RAMQD, F/2->B, Q/2->Q, Y=F */
	{'F', 1,'D','O','I', 0,'D','O','Z'}, /* RAMD, F/2->B, Y=F */
	{'F', 1,'U','I','O', 1,'U','I','O'}, /* RAMQU, F*2->B, Q*2->Q, Y=F */
	{'F', 1,'U','I','O', 0,'U','Z','O'} /* RAMU, F*2->B, Y=F */
};

typedef struct am2901_inst_t {
	/* Dual-ported RAM Addresses */
	nibble_t rA; /* 4-bit Register address in RAM to use for "A" (read) */
	nibble_t rB; /* 4-bit Register address in RAM to use for "B" (read/write) */

	/* Instruction word of 9 bits in 3 groups of 3 bit (octal) values */
	octal_t src; /* I210 - Source operand octal value */
	octal_t func; /* I543 - ALU function octal value */ 
	octal_t dest; /* I876 - Destination octal value */

} am2901_inst_t;

typedef struct am2901_shifter_t {
	bit_t	*msb, *lsb; /* Tristate external connections x3 and x0 for shifter */
	unsigned char mux; /* MUX state for this shifter */
	unsigned char msb_dir, lsb_dir; /* Directions for msb and lsb pins on this shifter */
} am2901_shifter_t;

typedef struct am2901_output_t {
	/* Outputs */
	/* Result Flags */
	bit_t *Cout; /* Carry-out */
	bit_t *OVR; /* Overflow flag */
	bit_t *FZ; /* Zero flag output */
	bit_t *F3; /* High bit set (Negitive) flag output */

	/* Look ahead carry signals */
	bit_t *P_; /* Propagate signal */
	bit_t *G_; /* Generate signal */

	/* Result */
	nibble_t *Y; /* Output value */

} am2901_output_t;

typedef struct am2901_input_t {
	/* Carry-in Input */
	bit_t *Cin; /* Carry-in */

	/* External data input */
	nibble_t *D; /* Direct data input */

	/* Output Enable */
	bit_t *OE_; /* Output Enable (Active LOW) HiZ=1 */


} am2901_input_t;

typedef struct am2901_device_t {
	clock_state_t *clk;
	char *id; /* Identifier for this device */

	/* Instruction Input */
	am2901_inst_t *inst;

	/* Input struct */
	am2901_input_t *in;

	/* RAM-Shifter */
	am2901_shifter_t *RAM_shift;

	/* Q-Shifter */
	am2901_shifter_t *Q_shift;

	/* Output struct */
	am2901_output_t *out;


	/* Internal State */

	/* RAM Register file */
	nibble_t RAM[16]; /* Register file with 16 x 4-bit words of RAM */
	nibble_t A; /* Value in A Latch */
	nibble_t B; /* Value in B Latch */
	bit_t RAM_WE; /* RAM data write enable */

	/* Q Register */
	nibble_t Q; /* Output data from Q Register */
	bit_t Q_EN; /* Q data write enable */

	/* Source MUXes */
	unsigned char Rmux; /* R Operand MUX select ('Z'ero (0),'D','A') */
	unsigned char Smux; /* S Operand MUX select ('Z'ero (0),'A','B','Q') */

	/* ALU Result */
	nibble_t F; /* Result from ALU */

	/* Output MUX */
	unsigned char Ymux; /* Y MUX output source select ('F' or 'A') */


} am2901_device_t;


int am2901_init(am2901_device_t *dev, char* id,
	clock_state_t *clk, /* Clock state from clockline */
	/* Instruction struct */
	am2901_inst_t *inst,

	/* Input struct */
	am2901_input_t *in;

	/* RAM input shifter */
	am2901_shifter_t *RAM_shift,
	/* Q input shifter */
	am2901_shifter_t *Q_shift,

	/* Output struct */
	am2901_output_t *out;
);

char *am2901_update(am2901_device_t *dev);
void am2901_print_state(am2901_device_t *dev);
void am2901_print_function_replace_RSC(char *fstr, char sym, char R, char S, char C);
