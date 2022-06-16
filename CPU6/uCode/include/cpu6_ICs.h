#include "types-common.h"
#include "siglist.h"

enum IC_TYPE { IC_DECODER, IC_MUX, IC_ADDR_LATCH, IC_REGISTER };

enum SRC_TYPE { SRC_SIG, SRC_FORCE, SRC_uIW, SRC_ClockB1_ };

typedef struct src_uIW_t {
	byte_t bit;
} src_uIW_t;

/* Enable pin types */
typedef struct en_pin_act_t {
	char *d; // Description
} en_pin_act_t;

typedef struct U_en_t {
	char *d; // Description
	en_pin_act_t LO, HI;
	byte_t bit;
	enum SRC_TYPE src; /* Source type ID */
	union {
		enum SIGS sig; /* Signal structure */
		byte_t force; /* Forced value */
		src_uIW_t uIW; /* uIW bit */
	};
} U_en_t;

typedef U_en_t U_en_list_t[];


/* Select/Address pin types */
typedef struct sel_pin_act_t {
	char *d; // Description
} sel_pin_act_t;

typedef struct U_sel_t {
	char *d; // Description
	sel_pin_act_t LO, HI;
	byte_t bit;
	enum SRC_TYPE src; /* Source type ID */
	union {
		enum SIGS sig; /* Signal structure */
		byte_t force; /* Forced value */
		src_uIW_t uIW; /* uIW bit */
	};
} U_sel_t;

typedef U_sel_t U_sel_list_t[];


/* Output pin types */
typedef struct out_pin_act_t {
	char *d; // Description
} out_pin_act_t;

typedef struct U_out_t {
	char *d; // Description
	out_pin_act_t LO, HI;
	enum SIGS sig; /* Signal structure */
} U_out_t;

typedef U_out_t U_out_list_t[];

/* Input pin types */
typedef struct in_pin_act_t {
	char *d; // Description
} in_pin_act_t;

typedef struct U_in_t {
	char *d; // Description
	in_pin_act_t LO, HI;
	byte_t bit;
	enum SRC_TYPE src; /* Source type ID */
	union {
		enum SIGS sig; /* Signal structure */
		byte_t force; /* Forced value */
		src_uIW_t uIW; /* uIW bit */
	};
} U_in_t;

typedef U_in_t U_in_list_t[];


typedef struct Up_t {
	char *name;
	byte_t num;
	char *type;
} Up_t;

typedef struct U_t {
	char *name;
	char *d; /* Description */
	char col;
	byte_t row;
	char unit;
	U_en_list_t *en;
	U_en_list_t *clr;
	U_sel_list_t *sel;
	U_in_list_t *in;
	U_out_list_t *out;
	enum IC_TYPE type;
	void *f;
} U_t;

typedef struct Us_t {
	char *name;
} Us_t;

typedef struct IC_decoder_t {
	Us_t	select;
	Us_t	enable;
	U_t	*latch;
} IC_decoder_t;

typedef struct IC_mux_t {
	Us_t	select;
	Us_t	enable;
	U_t	*latch;
} IC_mux_t;



typedef struct IC_addr_latch_t {
	Us_t enable;
	Us_t clear;
	nibble_t size;
	byte_t data;
} IC_addr_latch_t;


typedef struct IC_register_t {
	Us_t enable;
	Us_t clock;
	nibble_t size;
	byte_t data;
} IC_register_t;


typedef struct sig_readers_t {
	enum SIGS sig;
} sig_reader_t;

typedef struct sig_writers_t {
	enum SIGS sig;
} sig_writer_t;

/*
#define get_out(ic) \
	({U_out_t* out; switch (ic.type) { \
	 case IC_DECODER: out=((IC_decoder_t *)ic.f)->out; break;\
	 case IC_MUX: out=((IC_mux_t *)ic.f)->out; break;\
	 case IC_ADDR_LATCH: out=((IC_addr_latch_t *)ic.f)->out; break;\
	 case IC_REGISTER: out=((IC_register_t *)ic.f)->out; break;\
	 } out;} )
*/
#define IC_get_out(ic,n) \
	((*(ic.out))[n])
#define IC_get_in(ic,n) \
	((*(ic.in))[n])

/* Decoder UD2A - 74LS139 (shares bits with UD3) */
/* E_: uIW<3> */
/* A<1:0>=uIW<1:0> */
/* DP-Bus Reader Select 0 */
U_en_list_t D_UD2A_en={
	{ .bit=0, .src=SRC_uIW, .uIW={.bit=003}, .LO={.d="UD2A.E_"}},
	{}
};

U_sel_list_t D_UD2A_sel={
	{ .bit=0, .src=SRC_uIW, .uIW={.bit=000}, .d="UD2A.A0"},
	{ .bit=1, .src=SRC_uIW, .uIW={.bit=001}, .d="UD2A.A1"},
	{}
};

U_out_list_t D_UD2A_out={
	{ .sig=SIG_DP_READ_NSWAPR_, .LO={.d="D2.0 READ NIBBLE SWAP REGISTER -> (DP-Bus)"}}, // C12p1
	{ .sig=SIG_DP_READ_RF_, .LO={.d="D2.1 READ REGISTER FILE -> (DP-Bus)"}}, // D13p1
	{ .sig=SIG_DP_READ_IAB_MSB_, .LO={.d="D2.2 READ ADDRESS BUS MSB (A-Bus) -> (DP-Bus)"}}, // A6p7
	{ .sig=SIG_DP_READ_IAB_LSB_, .LO={.d="D2.3 READ ADDRESS BUS LSB (A-Bus)-> (DP-Bus)"}}, // A4p7
	{}
};
IC_decoder_t D_UD2A= {
};

U_t UD2A= {
	.name="D_UD2A_DP_READ_SEL0",
	.d="DP-Bus Reader Select Decoder 0",
	.col='D',
	.row=2,
	.unit='A',
	.en=&D_UD2A_en,
	.sel=&D_UD2A_sel,
	.out=&D_UD2A_out,
	.type=IC_DECODER,
	.f=(IC_decoder_t *) &D_UD2A
};

/* Decoder UD3 - 74LS138 (shares bits with UD2A) */
/* E1_,E2_=Tied LO; E3: uIW<3> */
/* A<2:0>=uIW<2:0> */
/* DP-Bus Reader Select 1 */
U_en_list_t D_UD3_en={
	{ .bit=0, .src=SRC_FORCE, .force=0, .LO={.d="UD3.E1_ (GND)"}},
	{ .bit=1, .src=SRC_FORCE, .force=0, .LO={.d="UD3.E2_ (GND)"}},
	{ .bit=2, .src=SRC_uIW, .uIW={.bit=003}, .HI={.d="UD3.E3"}},
	{}
};

U_sel_list_t D_UD3_sel={
	{ .bit=0, .src=SRC_uIW, .uIW={.bit=000}, .d="UD3.A0"},
	{ .bit=1, .src=SRC_uIW, .uIW={.bit=001}, .d="UD3.A1"},
	{ .bit=2, .src=SRC_uIW, .uIW={.bit=002}, .d="UD3.A2"},
	{}
};
U_out_list_t D_UD3_out={
	{ .sig=SIG_DP_READ_SYS_ABUS_EXT_, .LO={.d="D3.0 READ SYSTEM ADDRESS BUS EXTENDED -> (DP-Bus)"}}, // A9p7
	{ .sig=SIG_DP_READ_COND_LSN_,.LO={.d="D3.1 READ CONDITION LEAST SIGNIFICANT NIBBLE -> (DP-Bus)"}}, // M7p19
	{ .sig=SIG_DP_READ_SYS_DBUS_,.LO={.d="D3.2 READ SYSTEM DATA BUS (Bus.Sys.DATA) -> (DP-Bus)"}}, // A12p11
	{ .sig=SIG_DP_READ_IL_,.LO={.d="D3.3 READ INTERRUPT LEVEL REGISTER -> (DP-Bus)"}}, // A8p7
	{ .sig=SIG_DP_READ_COND_MSN_,.LO={.d="D3.4 READ CONDITION MOST SIGNIFICANT NIBBLE -> (DP-Bus)"}}, // M7p1
	{ .sig=SIG_DP_READ_uC_CONST_,.LO={.d="D3.5 READ uC DATA CONSTANT (uIW) -> (DP-Bus)"}}, // M6p18
	{}
};
IC_decoder_t D_UD3= {
};

U_t UD3= {
	.name="D_UD3_DP_READ_SEL1",
	.d="DP-Bus Reader Select Decoder 1",
	.col='D',
	.row=3,
	.en=&D_UD3_en,
	.sel=&D_UD3_sel,
	.out=&D_UD3_out,
	.type=IC_DECODER,
	.f=(IC_decoder_t *) &D_UD3
};


/* Decoder UE6 - 74LS138 */
/* E1_,E2_=Tied LO; E3: Tied HI */
/* A<2:0>=uIW<6:4> */
/* F-Bus Writer Select */
U_en_list_t D_UE6_en={
	{ .bit=0, .src=SRC_FORCE, .force=0, .LO={.d="UE6.E1_ (GND)"}},
	{ .bit=1, .src=SRC_FORCE, .force=0, .LO={.d="UE6.E2_ (GND)"}},
	{ .bit=2, .src=SRC_FORCE, .force=1, .HI={.d="UE6.E3 (+5)"}},
	{}
};

U_sel_list_t D_UE6_sel={
	{ .bit=0, .src=SRC_uIW, .uIW={.bit=004}, .d="UE6.A0"},
	{ .bit=1, .src=SRC_uIW, .uIW={.bit=005}, .d="UE6.A1"},
	{ .bit=2, .src=SRC_uIW, .uIW={.bit=006}, .d="UE6.A2"},
	{}
};
U_out_list_t D_UE6_out={
	{.LO={.d="E6.0 IDLE STATE"}},
	{.sig=SIG_F_WRITE_RR_, .LO={.d="E6.1 WRITE RESULT REGISTER (R-Bus) <- (F-Bus)"}}, // C9p1
	{.sig=SIG_F_WRITE_RIR_, .LO={.d="E6.2 WRITE REGISTER INDEX SELECTION REGISTER <- (F-Bus)"}}, // C13p1
	{.sig=SIG_F_WRITE_UD9_, .LO={.d="E6.3 UD9 ENABLE"}}, // D9p1
	{.sig=SIG_F_WRITE_PTBAR_, .LO={.d="E6.4 WRITE PAGE TABLE MAP REGISTER <- (F-Bus)"}},  // D11p1
	{.sig=SIG_WAR2MAR_,
		.LO={.d="E6.5 WRITE WORKING ADDRES REGISTER TO MEMORY ADDRESS REGISTER (WAR->MAR)<- (A-Bus)"},
		.HI={.d="E6.5 WORKING ADDRESS REGISTER SOURCE = RESULT <- (R-Bus)"}}, // [BC]{3,4}p1
	{.sig=SIG_F_WRITE_SEQ_AR_, .LO={.d="E6.6 WRITE DATA TO SEQUENCERS ADDRESS REGISTER <- (F-Bus)"}}, // AM2909p1 (RE_)
	{.sig=SIG_F_WRITE_M12_, .LO={.d="E6.7 (Load Condition Code Reg M12?)"}}, // M12p1
	{}
};
IC_decoder_t D_UE6={ // Decoder E6 (from latch E5)
};

U_t UE6= {
	.name="D_UD3_FR_WRITE_SEL0",
	.d="F-Bus/R-Bus Write Select Decoder 0",
	.col='E',
	.row=6,
	.en=&D_UE6_en,
	.sel=&D_UE6_sel,
	.out=&D_UE6_out,
	.type=IC_DECODER,
	.f=(IC_decoder_t *) &D_UE6
};

/* Decoder UK11 - 74LS138 */
/* E1_,E2_=ClockB1_; E3: Tied HI? */
/* A<2:0>=uIW<9:7> */
U_en_list_t D_UK11_en={
	{ .bit=0x0, .src=SRC_ClockB1_, .LO={.d="UK11.E1_ (ClockB1_)"}},
	{ .bit=0x1, .src=SRC_ClockB1_, .LO={.d="UK11.E2_ (ClockB1_)"}},
	{ .bit=0x2, .src=SRC_FORCE, .force=1, .HI={.d="UK11.E3 (+5)"}},
	{}
};

U_sel_list_t D_UK11_sel={
	{ .bit=0x0, .src=SRC_uIW, .uIW={.bit=007}, .d="UK11.A0"},
	{ .bit=0x1, .src=SRC_uIW, .uIW={.bit=010}, .d="UK11.A1"},
	{ .bit=0x2, .src=SRC_uIW, .uIW={.bit=011}, .d="UK11.A2"},
	{}
};
U_out_list_t D_UK11_out={
	// Decoder K11 & (K12C(NOR)/H13B(NAND4)) - Write Register File(0) and Clock output select (1-7)
	// D.UH13B, UE5.Q5 -> A0.UK11, UJ4.Q7 -> A1.UK11, UJ4.Q6 -> A2.UK11 All LO to Enable RF Write
	// WRITE REGFILE NAND4[UH13B]( K11.A2[B] (NOR[UK12C](K11.A0,K11.A1)[C,D] ) -> UD14.WE_/UD15.WE_
	// 
	// WRITE_RF logic tied to input signals of K11
	//{"K11.0 Write Register File <- (R-Bus)",""},
	// All output signals clocked through inverting input to inverting outputs (Idle HI)
	// Propagation delay ~15ns-40ns
	{.d="Write RF"},
	{.sig=SIG_UK11_CLK_UL13A_S_, .LO={.d="K11.1 (Unknown Clock Select) to flip-flop UL13A.S_ ->UM8.I1a (?HALT/SS?)"}},
	{.sig=SIG_UK11_CLK_UM13_E_, .LO={.d="K11.2 (M13 Gate?)"}},
	{.sig=SIG_UK11_CLK_UF11_E_, .LO={.d="K11.3 (F11 Enable)"}},
	{.sig=SIG_UK11_CLK_O4_, .LO={.d="K11.4 (Write Register Clock? Unknown Clock Selected)"}},
	{.sig=SIG_CLK_WrR_WAR_LSB_,
		.LO={.d="K11.5 CLOCKED WRITE TO WORKING ADDRESS REGISER LSB <- (IA-Bus/R-Bus) (Clock.WrR_WAR_LSB)"}},
	{.sig=SIG_CLK_RdL_SysDB_,
		.LO={.d="K11.6 CLOCKED READ FROM SYSTEM DATA BUS RECEIVE LATCH -> (DP-Bus) (Clock.RdL_SysDB)"}},
	{.sig=SIG_CLK_WrR_SysDB_,
		.LO={.d="K11.7 CLOCKED WRITE TO SYSTEM DATA BUS OUTPUT REGISTER <- (R-Bus) (Clock.WrR_SysDB)"}},
	{}
};
IC_decoder_t D_UK11={
};

U_t UK11= {
	.name="D_UK11_CLK_SEL1",
	.d="Clocked Output Select Decoder 1",
	.col='K',
	.row=11,
	.en=&D_UK11_en,
	.sel=&D_UK11_sel,
	.out=&D_UK11_out,
	.type=IC_DECODER,
	.f=(IC_decoder_t *) &D_UK11
};


/* Decoder UH11 - 74LS138 */
/* E1_,E2_: Tied GND; E3: Tied HI */
/* A<2:0>=uIW<12:10> (014:012)*/
U_en_list_t D_UH11_en={
	{ .bit=0, .src=SRC_FORCE, .force=0, .HI={.d="UH11.E1_ (GND)"}},
	{ .bit=1, .src=SRC_FORCE, .force=0, .HI={.d="UH11.E2_ (GND)"}},
	{ .bit=2, .src=SRC_FORCE, .force=1, .HI={.d="UH11.E3 (+5)"}},
	{}
};

U_sel_list_t D_UH11_sel={
	{ .bit=0, .src=SRC_uIW, .uIW={.bit=012}, .d="UH11.A0"},
	{ .bit=1, .src=SRC_uIW, .uIW={.bit=013}, .d="UH11.A1"},
	{ .bit=2, .src=SRC_uIW, .uIW={.bit=014}, .d="UH11.A2"},
	{}
};
U_out_list_t D_UH11_out={ // Decoder H11
	{.sig=SIG_UH11_O0_, .LO={.d="H11.0 (Unknown Output)"}},
	{.sig=SIG_UH11_O1_, .LO={.d="H11.1 (Unknown Output)"}},
	{.sig=SIG_UH11_O2_, .LO={.d="H11.2 (Unknown Output - ?DBE_ and WTIN?)"}},
	{.sig=SIG_WrR_WAR_MSB_,
		.LO={.d="H11.3 WRITE TO WORKING ADDRESS REGISER MSB <- (IA-Bus/R-Bus) (WrR_WAR_MSB)"}},
	{.sig=SIG_UH11_O4_, .LO={.d="H11.4 (Unknown Output)"}},

	{.sig=SIG_UH11_O5_, .LO={.d="H11.5 (Increment MAR Counter?)"}},
	{.sig=SIG_Rd_MAPROM_,
		.LO={.d="H11.6 READ MAPPING PROM -> (F-Bus)"},
		.HI={.d="H11.6 READ ALU RESULT -> (F-Bus)"}},
	{.sig=SIG_WrR_NSWAPR_, .LO={.d="H11.7 WRITE NIBBLE SWAP REGISTER <- (DP-Bus) (NSWAPR)"}},
	{}
};
IC_decoder_t D_UH11= {
};

U_t UH11= {
	.name="D_UH11_OUT_SEL0",
	.d="Output Select Decoder 0",
	.col='H',
	.row=11,
	.en=&D_UH11_en,
	.sel=&D_UH11_sel,
	.out=&D_UH11_out,
	.type=IC_DECODER,
	.f=(IC_decoder_t *) &D_UH11
};

/* Decoder UE7 - 74LS138 */
/* E1_,E2_: Tied LO; E3: BUS_RESET_ != LO; */
/* A<1:0>=uIW<14:13> (016:015); A<2>=LO*/
U_en_list_t D_UE7_en={
	{ .bit=0, .src=SRC_FORCE, .force=0, .LO={.d="UE7.E1_ (GND)"}},
	{ .bit=1, .src=SRC_FORCE, .force=0, .LO={.d="UE7.E2_ (GND)"}},
	{ .bit=2, .src=SRC_SIG, .sig=SIG_BUS_RESET_NEXT_, .HI={.d="UE7.E3 (!BUS_RESET_)"}},
	{}
};

U_sel_list_t D_UE7_sel={
	{ .bit=0, .src=SRC_uIW, .uIW={.bit=015}, .d="UE7.A0"},
	{ .bit=1, .src=SRC_uIW, .uIW={.bit=016}, .d="UE7.A1"},
	{ .bit=2, .src=SRC_FORCE, .force=0, .d="UE7.A2 (GND)"},
	{}
};
U_out_list_t D_UE7_out={ // Decoder E7
	{.LO={.d="E7.0 LO"}},
	{.sig=SIG_UE7_ClockB_E8A_ENABLE_,
		.LO={.d="E7.1 ->UE8A.B (UE8A[NOR](ClockB2_,UE7.O1) -> (ClockB2_EA8) -> UE11A.A[NAND3], UE14A.D[FF-74LS74] )"}},
	{.sig=SIG_UE7_WrR_FLR_,.LO={.d="E7.2 WRITE FLAG REGISTER"}},
	{.sig=SIG_UE7_O3_MESS_,
		.LO={.d="E7.3 ->UE8D.B (UE8D[NOR]((=UD8A.B[NAND4]->UD7.2S1_, =UD8B.C[NAND4]->UD7.1S1_, =UD10B.A[NOR]), UE7.O3)"}},
	{}
};

IC_decoder_t D_UE7={
};

U_t UE7= {
	.name="D_UE7_CLK_SEL1",
	.d="Bus Control Decoder 0",
	.col='E',
	.row=7,
	.en=&D_UE7_en,
	.sel=&D_UE7_sel,
	.out=&D_UE7_out,
	.type=IC_DECODER,
	.f=(IC_decoder_t *) &D_UE7
};

/* Addressable Latch UF11 - 74LS259, Gated clock from K11.3 drives enable E_ */
/* UD4.Q1 drives Clr_ */
/* A<2:0>=uIW<46:44> (056:054); D=uIW<43> (053) */
U_en_list_t AL_UF11_en={
	{ .bit=0, .src=SRC_SIG, .sig=SIG_UK11_CLK_UF11_E_, .LO={.d="UF11.E_ (Clock from UK11)"}},
	{}
};

U_en_list_t AL_UF11_clr={
	{ .bit=0, .src=SRC_SIG, .sig=SIG_BUSRESET_, .LO={.d="UF11.Clr_ (BUSRESET_ from UD4.Q1)"}},
	{}
};

U_sel_list_t AL_UF11_sel={
	{ .bit=0, .src=SRC_uIW, .uIW={.bit=054}, .d="UF11.A0"},
	{ .bit=1, .src=SRC_uIW, .uIW={.bit=055}, .d="UF11.A1"},
	{ .bit=2, .src=SRC_uIW, .uIW={.bit=056}, .d="UF11.A1"},
	{}
};
U_in_list_t AL_UF11_in={
	{ .bit=0, .src=SRC_uIW, .uIW={.bit=053}, .d="UF11.D"},
	{}
};
U_out_list_t AL_UF11_out={
	{.d="UF11.Q0"},
	{ .LO={.d="UF11.Q1 (ABE_) Address Bus Enable (Active LO) "}},
	{.d="UF11.Q2"},
	{.d="UF11.Q3"},
	{.d="UF11.Q4 UM8.I0b"},
	{.d="UF11.Q5"},
	{.d="UF11.Q6"},
	{.d="UF11.Q7"},
	{}
};
IC_addr_latch_t AL_UF11={
};

U_t UF11= {
	.name="AL_UF11_BUS_CTL",
	.d="Bus Control Addressable Latch 0",
	.col='F',
	.row=11,
	.en=&AL_UF11_en,
	.clr=&AL_UF11_clr,
	.sel=&AL_UF11_sel,
	.in=&AL_UF11_in,
	.out=&AL_UF11_out,
	.type=IC_ADDR_LATCH,
	.f=(IC_addr_latch_t *) &AL_UF11
};


/* UH6 - Shift Control *
 * Select from 
 * Output enables driven by ALU.I7 -> OEa_, -> INV -> OEb_
 * ALU.I7=0 -> Shift Down, ALU.I7=1 Shift Up
 * Shifter input connections:
 *                         SHCS=0     SHCS=1     SHCS=2     SHCS=3
 * ALU.I7=0: (Za Enabled)  I0a=S(2b)  I1a=?(1b)  I2a=Q0     I3a=C
 * ALU.I7=1: (Zb Enabled)  I0b=RAM7   I1b=?(1a)  I2b=S(0a)  I3b=
 *
 * Shifter output connections:
 * Za -> RAM7[ALU1.RAM3] (UF9), UJ10.I5
 * Zb -> Q0[ALU0.Q0] (UF7), UJ10.I7
 *
 * 0,0:    S->RAM7  SRA (Sign extend S->MSB)
 * 0,1:    ?->RAM7  SRL? (Zero?)
 * 0,2:   Q0->RAM7  (SRA/RRR) Half-word (Q0 of High byte shifts into MSB of Low byte)
 * 0,3:    C->RAM7  RRR (Rotate carry into MSB)
 *
 * 1,0: RAM7->Q0    RLR/SLR Half-word? (RAM7->Q0)
 * 1,1:    ?->Q0    SLR? (Zero?)
 * 1,2:    S->Q0    ?
 * 1,3:    ?->Q0    (C?) RLR?
 *
 *
 * See microcode @ 0x0d2-0x0e2 for 16 bit shift up through Q
 */

/* MUX UH6 - 74LS253 (Tristate OUT) */
/* Ea_: uIW<41> (051) (uIW.ALU.I<7> Shift Dn); Eb_: INV(uIW<41>) (INV(uIW.ALU.I<7>) Shift Up) */
/* S<1:0>=uIW<52:51> (064:063)*/

/* UH6 Common */
U_sel_list_t M_UH6_sel={
	{ .bit=0, .src=SRC_uIW, .uIW={.bit=063}, .d="UH6.A0"},
	{ .bit=1, .src=SRC_uIW, .uIW={.bit=064}, .d="UH6.A1"},
	{}
};

/* UH6a */
U_en_list_t M_UH6a_en={
	{ .bit=0, .src=SRC_uIW, .uIW={.bit=051}, .LO={.d="UH6.Ea_ Shifter Dn"}},
	{}
};
U_out_list_t M_UH6a_out={
	{.sig=SIG_ALU_RAM7, .d="UH6.Za Shifter Down-line to RAM7 of ALU"},
	{}
};
U_in_list_t M_UH6a_in={
	{.sig=SIG_ALU_SIGN, .d="UH6.I0a Shifter Dn(0): SIGN->RAM7"},
	{.sig=SIG_LO, .d="UH6.I1a Shifter Dn(1): (0?)->RAM7"},
	{.sig=SIG_ALU_Q0, .d="UH6.I2a Shifter Dn(2): Q0->RAM7"},
	{.sig=SIG_ALU_Cout, .d="UH6.I3a Shifter Dn(3): Cout->RAM7"},
	{}
};
IC_mux_t M_UH6a={
};
U_t UH6a= {
	.name="M_UH6a_SHIFTER_DN_SRC",
	.d="Shifter Down-line Source Select MUX",
	.col='H',
	.row=6,
	.unit='a',
	.en=&M_UH6a_en,
	.sel=&M_UH6_sel,
	.in=&M_UH6a_in,
	.out=&M_UH6a_out,
	.type=IC_MUX,
	.f=(IC_mux_t *) &M_UH6a
};


/* UH6b */
U_en_list_t M_UH6b_en={
	{ .bit=0, .src=SRC_uIW, .uIW={.bit=051}, .HI={.d="[INV->]UH6.Eb_ Shifter Up"}},
	{}
};
U_out_list_t M_UH6b_out={
	{.sig=SIG_ALU_RAM7, .d="UH6.Zb Shifter Up-line input to Q0 of ALU"},
	{}
};
U_in_list_t M_UH6b_in={
	{.sig=SIG_ALU_RAM7, .d="UH6.I0b Shifter Up(0): RAM7->Q0"},
	{.sig=SIG_LO, .d="UH6.I1b Shifter Up(1): (0?)->Q0"},
	{.sig=SIG_ALU_SIGN, .d="UH6.I2b Shifter Up(2): SIGN->Q0"},
	{.sig=SIG_ALU_Cout, .d="UH6.I3b Shifter Up(3): ?Cout->Q0"},
	{}
};
IC_mux_t M_UH6b={
};
U_t UH6b= {
	.name="M_UH6b_SHIFTER_UP_SRC",
	.d="Shifter Up-line Source Select MUX",
	.col='H',
	.row=6,
	.unit='b',
	.en=&M_UH6b_en,
	.sel=&M_UH6_sel,
	.in=&M_UH6b_in,
	.out=&M_UH6b_out,
	.type=IC_MUX,
	.f=(IC_mux_t *) &M_UH6b
};


/* UF6 - Carry Control *
 * Select from 
 * Output enables driven by ?
 * Carry control input connections:
 *                         SHCS=0    SHCS=1     SHCS=2    SHCS=3
 *     ??=0: (Za Enabled)  I0a=?     I1a=?      I2a=?     I3a=?
 *     ??=1: (Zb Enabled)  I0b=?     I1b=?      I2b=?     I3b=?
 *
 * Carry control output connections:
 *  Za -> ALU0.Cn (UF7)
 *
 *  0,0 -> ? Zero?
 *  0,1 -> ? One?
 *  0,2 -> ? Carry?
 *  0,3 -> ? ??
 */

/* MUX UF6 - 74LS153 */
/* E_:? */
/* S<1:0>=uIW<52:51> (063:064) */

/* UF6 Common */
U_sel_list_t M_UF6_sel={
	{ .bit=0, .src=SRC_uIW, .uIW={.bit=063}, .d="UF6.A0"},
	{ .bit=1, .src=SRC_uIW, .uIW={.bit=064}, .d="UF6.A1"},
	{}
};

/* UF6a */
U_en_list_t M_UF6a_en={
	{ .bit=0, .src=SRC_FORCE, .force=0, .LO={.d="UF6.Ea_ Carry-Control"}},
	{}
};
U_out_list_t M_UF6a_out={
	{.sig=SIG_ALU_Cin, .d="UF6.Za Input to Carry-in of ALU"},
	{}
};
U_in_list_t M_UF6a_in={
	{.sig=SIG_LO, .d="UF6.I0a ?0->Cin"},
	{.sig=SIG_HI, .d="UF6.I1a ?1->Cin"},
	{.sig=SIG_ALU_Cout, .d="UF6.I2a ?C->Cin"},
	{.d="UF6.I3a ?->Cin"},
	{}
};
IC_mux_t M_UF6a={
};
U_t UF6a= {
	.name="M_UF6a_Cin_SRC",
	.d="ALU Carry-in Source Select MUX",
	.col='F',
	.row=6,
	.unit='a',
	.en=&M_UF6a_en,
	.sel=&M_UF6_sel,
	.in=&M_UF6a_in,
	.out=&M_UF6a_out,
	.type=IC_MUX,
	.f=(IC_mux_t *) &M_UF6a
};


/* MUX UJ12 - 74LS153 */
/* Ea_= ???, Eb_= ???; S<1:0>=uADDR<1:0>(<17:16>) (021:020)  */

/* UJ12 Common */
U_sel_list_t M_UJ12_sel={
	{ .bit=0, .src=SRC_uIW, .uIW={.bit=020}, .d="UJ12.A0"},
	{ .bit=1, .src=SRC_uIW, .uIW={.bit=021}, .d="UJ12.A1"},
	{}
};

/* UJ12a */
U_en_list_t M_UJ12a_en={
	{ .bit=0, .LO={.d="UJ12.Ea_"}},
	{}
};
U_in_list_t M_UJ12a_in= {
	{.d="UJ12.I0a"},
	{.sig=SIG_FLR_SIGN, .d="UJ12.I1a (=I3a) ALU Result F7 =? 1 (Sign: FLAG_M)"},
	{.d="UJ12.I2a R-Bus<1> or R-Bus<5>"},
	{.sig=SIG_FLR_SIGN, .d="UJ12.I3a (=I1a) ALU Result F7 =? 1 (Sign: FLAG_M)"},
	{}
};
U_out_list_t M_UJ12a_out={
	{.d="UJ12.Oa"},
	{}
};
IC_mux_t M_UJ12a= {
};

U_t UJ12a= {
	.name="M_UJ12a_COND_MUX",
	.d="UJ12a Condition Source Select MUX",
	.col='J',
	.row=12,
	.unit='a',
	.en=&M_UJ12a_en,
	.sel=&M_UJ12_sel,
	.in=&M_UJ12a_in,
	.out=&M_UJ12a_out,
	.type=IC_MUX,
	.f=(IC_mux_t *) &M_UJ12a
};


/* UJ12b */
U_en_list_t M_UJ12b_en={
	{ .bit=0, .LO={.d="UJ12.Eb_"}},
	{}
};
U_in_list_t M_UJ12b_in= {
	{.d="UJ12.I0b"},
	{.sig=SIG_FLR_ZERO, .d="UJ12.I1b (=uJ13.I0b) ALU Result F =? 0 (Zero; FLAG_V)"},
	{.d="UJ12.I2b"},
	{.d="UJ12.I3b UK10D.Y[as INV] <-UK10A.Y[NAND](FLA.D5, FLA.Q5"},
	{}
};
U_out_list_t M_UJ12b_out={
	{.d="UJ12.Ob"},
	{}
};
IC_mux_t M_UJ12b= {
};

U_t UJ12b= {
	.name="M_UJ12b_COND_MUX",
	.d="UJ12b Condition Source Select MUX",
	.col='J',
	.row=12,
	.unit='b',
	.en=&M_UJ12b_en,
	.sel=&M_UJ12_sel,
	.in=&M_UJ12b_in,
	.out=&M_UJ12b_out,
	.type=IC_MUX,
	.f=(IC_mux_t *) &M_UJ12b
};

/* MUX UK9 - 74LS151 */
/* E_: uIW.LA<0> (uIW<47> (057)); S<2:0>=uADDR<2:0> (uIW<18:16> (022:020)) */
/* Z_: Sequencer Override NANDs: UL6A.B, UL6D.B, UL6B.B, UK6A.B, UK6B.B, UK6C.A */
/*     Force JSR Di (Seq.FE_=0, Seq0.S0=1, Seq0.S1=1, Seq1.S0=1, Seq1.S1=1, Seq2.S0=1, Seq2.S1=1) */
U_en_list_t M_UK9_en={
	{ .bit=0, .src=SRC_uIW, .uIW={.bit=057}, .LO={.d="UK9.E_ Force Seq JSR Di "}},
	{}
};
U_sel_list_t M_UK9_sel={
	{ .bit=0, .src=SRC_uIW, .uIW={.bit=020}, .d="UK9.A0"},
	{ .bit=1, .src=SRC_uIW, .uIW={.bit=021}, .d="UK9.A1"},
	{ .bit=2, .src=SRC_uIW, .uIW={.bit=022}, .d="UK9.A2"},
	{}
};
U_in_list_t M_UK9_in= {
	{.d="UK9.I0"},
	{.d="UK9.I1"},
	{.d="UK9.I2"},
	{.d="UK9.I3"},
	{.d="UK9.I4 Socket P10p8 (Unpopulated)"},
	{.d="UK9.I5"},
	{.d="UK9.I6"},
	{.d="UK9.I7"},
	{}

};
U_out_list_t M_UK9_out={
	{.d="UK9.Z"},
	{.d="UK9.Z_ (-> Sequencer Force JSR)"},
	{}
};
IC_mux_t M_UK9= {
};
U_t UK9= {
	.name="M_UK9_FORCE_BRANCH_COND_MUX",
	.d="UK9 Force Branch Condition Select MUX",
	.col='K',
	.row=9,
	.en=&M_UK9_en,
	.sel=&M_UK9_sel,
	.in=&M_UK9_in,
	.out=&M_UK9_out,
	.type=IC_MUX,
	.f=(IC_mux_t *) &M_UK9
};

/* MUX UJ11 - 74LS151 */
/* E_: uADDR<2> (uIW<18> (022)); S<1:0>=uADDR<4:3> (uIW<20:19> (024:023)),S2=Flag_M(UJ9.Q2) */
/* Z: -> UM12p6 */
U_en_list_t M_UJ11_en={
	{ .bit=0, .src=SRC_uIW, .uIW={.bit=022}, .LO={.d="UJ11.E_ "}},
	{}
};
U_sel_list_t M_UJ11_sel={
	{ .bit=0, .src=SRC_uIW, .uIW={.bit=023}, .d="UJ11.A0"},
	{ .bit=1, .src=SRC_uIW, .uIW={.bit=024}, .d="UJ11.A1"},
	{ .bit=2, .src=SRC_SIG, .sig=SIG_FLR_SIGN, .d="UJ11.A2 Flag_M(UJ9.Q2)"},
	{}
};
U_in_list_t M_UJ11_in= {
	{.d="UJ11.I0 (=I4?)"},
	{.d="UJ11.I1"},
	{.d="UJ11.I2 UM12.Q2?"},
	{.d="UJ11.I3"},
	{.d="UJ11.I4 (=I0?)"},
	{.d="UJ11.I5"},
	{.d="UJ11.I6"}, 
	{.d="UJ11.I7"},
	{}
};
U_out_list_t M_UJ11_out={
	{.d="UJ11.Z (->UM12.D2)"},
	{.d="UJ11.Z_"},
	{}
};
IC_mux_t M_UJ11= {
};
U_t UJ11= {
	.name="M_UJ11_COND_SIGN",
	.d="UJ11 Sign Condition MUX",
	.col='J',
	.row=11,
	.en=&M_UJ11_en,
	.sel=&M_UJ11_sel,
	.in=&M_UJ11_in,
	.out=&M_UJ11_out,
	.type=IC_MUX,
	.f=(IC_mux_t *) &M_UJ11
};

/* MUX UJ13 - 74LS153 */
/* Ea_=Eb_= CASE_ (uIW<33> (041)); S<1:0>=uADDR<5:4> (uIW<21:20> (025:024))*/
/* Za: OR0; Zb: OR1 */

/* UJ13 Common */
U_sel_list_t M_UJ13_sel={
	{ .bit=0, .src=SRC_uIW, .uIW={.bit=020}, .d="UJ13.A0"},
	{ .bit=1, .src=SRC_uIW, .uIW={.bit=021}, .d="UJ13.A1"},
	{}
};

U_en_list_t M_UJ13_en={
	{ .bit=0, .src=SRC_uIW, .uIW={.bit=041}, .LO={.d="UJ13.Ea_,UJ13.Eb_ (CASE_) Enable OR0, OR1 conditionals"}},
	{}
};

/* UJ13a */
U_in_list_t M_UJ13a_in= {
	{.d="UJ13.I0a"},
	{.d="UJ13.I1a ALU Result Half-Carry"},
	{.d="UJ13.I2a"},
	{.d="UJ13.I3a"},
	{}
};
U_out_list_t M_UJ13a_out={
	{.d="UJ13.Oa Seq OR0"},
	{}
};
IC_mux_t M_UJ13a= {
};
U_t UJ13a= {
	.name="M_UJ13a_OR0_COND_MUX",
	.d="UJ13a Seq OR0 Condition Source Select MUX",
	.col='J',
	.row=13,
	.unit='a',
	.en=&M_UJ13_en,
	.sel=&M_UJ13_sel,
	.in=&M_UJ13a_in,
	.out=&M_UJ13a_out,
	.type=IC_MUX,
	.f=(IC_mux_t *) &M_UJ13a
};

U_in_list_t M_UJ13b_in= {
	{.d="UJ13.I0a ALU Result F =? 0 (Zero; FLAG_V)"},
	{.d="UJ13.I1b"},
	{.d="UJ13.I2b UB12A.A"},
	{.d="UJ13.I3b"},
	{}

};
U_out_list_t M_UJ13b_out={
	{.d="UJ13.Ob Seq OR1"},
	{}
};
IC_mux_t M_UJ13b= {
};

U_t UJ13b= {
	.name="M_UJ13b_OR1_COND_MUX",
	.d="UJ13b Seq OR1 Condition Source Select MUX",
	.col='J',
	.row=13,
	.unit='b',
	.en=&M_UJ13_en,
	.sel=&M_UJ13_sel,
	.in=&M_UJ13b_in,
	.out=&M_UJ13b_out,
	.type=IC_MUX,
	.f=(IC_mux_t *) &M_UJ13b
};

/* MUX UK13 - 74LS153 */
/* Ea_=Eb_= CASE_ (uIW<33> (041)); S<1:0>=uADDR<7:6> (uIW<23:22> (027:026))*/
/* UK13 Common */
U_sel_list_t M_UK13_sel={
	{ .bit=0, .src=SRC_uIW, .uIW={.bit=022}, .d="UK13.A0"},
	{ .bit=1, .src=SRC_uIW, .uIW={.bit=023}, .d="UK13.A1"},
	{}
};

U_en_list_t M_UK13_en={
	{ .bit=0, .src=SRC_uIW, .uIW={.bit=041}, .LO={.d="UK13.Ea_,UK13.Eb_ (CASE_) Enable OR2, OR3 conditionals"}},
	{}
};

/* UK13a */
U_in_list_t M_UK13a_in= {
	{.d="UK13.I0a UM12.Q3 (=UJ10.I0)"},
	{.d="UK13.I1a UF13.Q0 (Latches UB12p8) (=UJ10.I0, UK10C.[AB])"},
	{.d="UK13.I2a UF13.Q1"},
	{.d="UK13.I3a"},
	{}
};
U_out_list_t M_UK13a_out={
	{.d="UK13.Oa ?Seq OR2?"},
	{}
};
IC_mux_t M_UK13a= {
};
U_t UK13a= {
	.name="M_UK13a_OR2_COND_MUX",
	.d="UK13a Seq OR2? Condition Source Select MUX",
	.col='K',
	.row=13,
	.unit='a',
	.en=&M_UK13_en,
	.sel=&M_UK13_sel,
	.in=&M_UK13a_in,
	.out=&M_UK13a_out,
	.type=IC_MUX,
	.f=(IC_mux_t *) &M_UK13a
};

/* UK13b */
U_in_list_t M_UK13b_in= {
	{.d="UK13.I0a"},
	{.d="UK13.I1b UF13.Q4 (Latches UF14D.Y[as INV] <- UF14A.Y[NAND](DIPSW6(=UM7.I1a),=UH14.I1))"},
	{.d="UK13.I2b UF13.Q3 (Latches UE10p6)"},
	{.d="UK13.I3b UK10D.Y"},
	{}
};
U_out_list_t M_UK13b_out={
	{.d="UK13.Ob ?Seq OR3?"},
	{}
};
IC_mux_t M_UK13b= {
};

U_t UK13b= {
	.name="M_UK13b_OR3_COND_MUX",
	.d="UK13b Seq OR3? Condition Source Select MUX",
	.col='K',
	.row=13,
	.unit='b',
	.en=&M_UK13_en,
	.sel=&M_UK13_sel,
	.in=&M_UK13b_in,
	.out=&M_UK13b_out,
	.type=IC_MUX,
	.f=(IC_mux_t *) &M_UK13b
};

/* MUX UJ10 - 74LS151 */
/* E_: uADDR<5> (uIW<21> (025)); S<1:0>=uADDR<7:6> (uIW<23:22> (027:026)); S<2>=?*/
U_en_list_t M_UJ10_en={
	{ .bit=0, .src=SRC_uIW, .uIW={.bit=025}, .LO={.d="UJ10.E_ "}},
	{}
};
U_sel_list_t M_UJ10_sel={
	{ .bit=0, .src=SRC_uIW, .uIW={.bit=026}, .d="UJ10.A0"},
	{ .bit=1, .src=SRC_uIW, .uIW={.bit=027}, .d="UJ10.A1"},
	{ .bit=2, .d="UJ10.A2 Flag_? SRC?"},
	{}
};
U_in_list_t M_UJ10_in= {
	{.d="UJ10.I0 UM12.Q3 (=UK13.I0a, =UK10C.[AB])"},
	{.d="UJ10.I1 UK10C.Y (=INV(UJ10.I0) )"},
	{.d="UJ10.I2 Link Flag? (Carry)"},
	{.d="UJ10.I3"},
	{.d="UJ10.I4 R-Bus<3> or R-Bus<7>"},
	{.d="UJ10.I5 ALU.RAM7 (Shifter DownLine)"},
	{.d="UJ10.I6 ALU.RAM0"},
	{.d="UJ10.I7 ALU.Q0 (Shifter UpLine)"},
	{}
};
U_out_list_t M_UJ10_out={
	{.d="UJ10.Z"},
	{.d="UJ10.Z_"},
	{}
};
IC_mux_t M_UJ10= {
};
U_t UJ10= {
	.name="M_UJ10",
	.d="UJ10 MUX",
	.col='J',
	.row=10,
	.en=&M_UJ10_en,
	.sel=&M_UJ10_sel,
	.in=&M_UJ10_in,
	.out=&M_UJ10_out,
	.type=IC_MUX,
	.f=(IC_mux_t *) &M_UJ10
};




// sed -ne 's/U_t \(U.*\)=.*/\t\&\1,/p' ../include/cpu6_ICs.h >> ../include/cpu6_ICs.h
U_t *ICs[] = {
	&UD2A,
	&UD3,
	&UE6,
	&UK11,
	&UH11,
	&UE7,
	&UF11,
	&UH6a,
	&UH6b,
	&UF6a,
	&UJ12a,
	&UJ12b,
	&UK9,
	&UJ11,
	&UJ13a,
	&UJ13b,
	&UK13a,
	&UK13b,
	&UJ10,
	0
};
