#include "CPU6-ISA.h"
#include "CPU6-machine.h"
#include "ginsumatic.h"


typedef struct BUS_AB_CPU6_t {
	uint16_t address;
	uint32_t address_ext;
} BUS_AB_CPU6_t;

typedef struct BUS_AB_t {
	uint16_t address;
} BUS_AB_t;

typedef union BUS_DB_t {
	uint16_t paritydata;
	struct {
		uint8_t data;
		uint8_t parity;
	};
	struct {
		uint16_t data_bits:8;
		uint16_t parity_bit:1;
	};

} BUS_DB_t;

typedef struct BUS_serial_priority_t {
	uint8_t I;
	uint8_t O;
} BUS_serial_priority_t;

typedef union BUS_IRQ_t {
	uint16_t IRQ_lines;
	struct {
		uint16_t CL_:4; /* Current IL */
		uint16_t INTR:1; /* IRQ Signal */
		uint16_t INR_:4; /* IRQ Number */
	} sig;
} BUS_IRQ_t;

typedef struct BUS_DMA_t {
	uint8_t DMA_lines;
	struct {
		uint8_t IORQ:1; /* DMA Request */
		uint8_t IACK:1; /* DMA ACK from CPU */
		uint8_t IDON:1; /* DMA Complete */
		uint8_t IOBY:1; /* DMA ACK Complete */
		uint8_t :2; /* Unused bits */
	} sig;
} BUS_DMA_t;

typedef union BUS_CTRL_t {
	uint8_t CTRL_lines;
	struct {
		uint8_t RDIN:1; /* Initiate Read */	
		uint8_t WTIN:1; /* Initiate Write */
		uint8_t DSYN:1; /* Synchronous Device */
		uint8_t BUSY:1; /* Memory or I/O Busy */
		uint8_t DRDY:1; /* Data Ready */
		uint8_t APRE:1; /* Address Preempted */
		uint8_t PERR:1; /* Parity Error */
		uint8_t MRST:1; /* Master Reset */
	} sig;
} BUS_CTRL_t;

typedef struct BUS_CPU6_t {
	/* Address Bus (full address space) */
	/* Old/MMIO devices use A15=(A15&A16&A17) */
	BUS_AB_CPU6_t *AB;
	BUS_DB_t *DB;
	BUS_CTRL_t *CTRL;
	BUS_IRQ_t *IRQ;
	BUS_serial_priority_t *INP;
	BUS_DMA_t *DMA;
	BUS_serial_priority_t *IOP;

} BUS_CPU6_t;

typedef BUS_CPU6_t SYS_BUS_t;

#define EXT_TO_COMPAT_ADDRESS(address_ext) ( address_ext & ( ( ( address_ext & 0x38000) == 0x38000 )? 0xffff : 0x7fff ) )
#define BUS_CPU6_ADDRESS_EXT(bus) ( bus->AB->address_ext )
#define BUS_CPU6_ADDRESS_COMPAT(bus) ( EXT_TO_COMPAT_ADDRESS(BUS_ADDRESS_EXT(bus)) )
#define BUS_ADDRESS_EXT(bus) BUS_CPU6_ADDRESS_EXT(bus)
#define BUS_ADDRESS(bus) BUS_CPU6_ADDRESS_COMAT(bus)
#define BUS_ADDRESS_MMIO(bus) BUS_CPU6_ADDRESS_COMPAT(bus)
#define ADDRESS_IN_RANGE(addr,base,size) ( ( addr >= base ) && ( addr < (base + size) ) )
#define ADDRESS_OFFSET(addr,base) ( ( addr - base ) )
#define BUS_DATA_PARITY_CHECK(bus) ( PARITY_BYTE(bus->DB.data) ^ bus->DB.parity )

#define BUS_ASSERT_BIT(sig) sig = 1
#define BUS_IS_BIT_ASSERTED(sig) (sig?1:0);

int BUS_assert_MRST(BUS_CPU6_t *bus);

int CPU6_BUS_assert_AB(BUS_CPU6_t *bus, uint32_t address) {
	bus->AB->address_ext = address;
	bus->AB->address = EXT_TO_COMPAT_ADDRESS(address);
}

int BUS_assert_AB(BUS_CPU6_t *bus, uint16_t address, uint32_t address_ext) {
	bus->AB->address = address | EXT_TO_COMPAT_ADDRESS(address_ext);
	bus->AB->address_ext = address | address_ext;
}

int BUS_assert_DB(BUS_CPU6_t *bus, uint8_t data) {
	bus->DB->data = data;
	bus->DB->parity = PARITY_BYTE(data);	
}

int BUS_assert_RDIN(BUS_CPU6_t *bus) {
	BUS_ASSERT_BIT(bus->CTRL->sig.RDIN);
}

int BUS_assert_WTIN(BUS_CPU6_t *bus) {
	BUS_ASSERT_BIT(bus->CTRL->sig.WTIN);
}


int BUS_assert_IACK(BUS_CPU6_t *bus);
int BUS_assert_IOBY(BUS_CPU6_t *bus);
int BUS_assert_CL(BUS_CPU6_t *bus);

enum BUS_OPS {
	BUS_OP_READ,
	BUS_OP_WRITE
};

enum BUS_CYCLE_STATES {
	BUS_CYCLE_IDLE,
	BUS_CYCLE_MEM_BEGIN_WAIT_CLOCK,
	BUS_CYCLE_MEM_BEGIN,
	BUS_CYCLE_MEM_MAYBE_ASSERT_APRE,
	BUS_CYCLE_MEM_CHECK_APRE,
	BUS_CYCLE_MEM_WAIT_INIT,
	BUS_CYCLE_MEM_WAIT_BUSY_CLEAR
};


int BUS_MEM_cycle_initiate(BUS_CPU6_t *bus, enum BUS_CYCLE_STATES state, enum BUS_OPS bus_op  ) {

	switch(state) {
		case BUS_CYCLE_IDLE:
			return(BUS_CYCLE_MEM_BEGIN_WAIT_CLOCK);
		case BUS_CYCLE_MEM_BEGIN_WAIT_CLOCK:
			return(BUS_CYCLE_MEM_BEGIN);
		case BUS_CYCLE_MEM_BEGIN:
			switch(bus_op) { 
				case BUS_OP_WRITE:
					BUS_assert_WTIN(bus);
					return(BUS_CYCLE_MEM_WAIT_BUSY_CLEAR);
				case BUS_OP_READ:
					BUS_assert_RDIN(bus);
					return(BUS_CYCLE_MEM_WAIT_BUSY_CLEAR);
			};
		default: break;
	}
}


int BUS_MEM_cycle_respond(BUS_CPU6_t *bus, enum BUS_CYCLE_STATES state, enum BUS_OPS bus_op  ) {
	switch(state) {
		/* Our address has been asserted, deal with APRE preemption */
		case BUS_CYCLE_MEM_MAYBE_ASSERT_APRE:
		case BUS_CYCLE_MEM_CHECK_APRE:
			
		case BUS_CYCLE_MEM_WAIT_INIT: 
		case BUS_CYCLE_MEM_BEGIN_WAIT_CLOCK:
			
	}
}


int CPU6_BUS_check_IRQ(BUS_CPU6_t *bus) {
	if(bus->IRQ->sig.INTR) {
		uint8_t CL = (~bus->IRQ->sig.CL_)&0xf;
		uint8_t INR = (~bus->IRQ->sig.INR_)&0xf;
		printf("Interrupt 0x%0x requested, current LVL is 0x%0x\n", INR, CL );
		if( CL <  INR ) {
			printf("Interrupt would occur (not yet implemented)\n");
			return(1);
		} else {
			printf("Interrupt request invalid? - priority inversion\n");
			return(-1);
		}
	}
	return(0);
}

int CPU6_BUS_check_DMA(BUS_CPU6_t *bus) {
	if(bus->DMA->sig.IORQ) {
		printf("DMA requested (but not yet implemented)\n");
		return(1);
	}
	return(0);
}

uint8_t CPU6_BUS_sm(BUS_CPU6_t *bus, enum BUS_CYCLE_STATES state) {
	uint8_t bus_op=0;
	uint32_t address=0;
	uint8_t data=0;
	switch(state) {
		case BUS_CYCLE_IDLE:
			CPU6_BUS_check_IRQ(bus);
			CPU6_BUS_check_DMA(bus);
			break;
	
			switch(bus_op) {
				case BUS_OP_WRITE: BUS_assert_DB(bus,data); /* Falling through is not an accident */
				case BUS_OP_READ:
					CPU6_BUS_assert_AB(bus,address);
					BUS_MEM_cycle(bus,state,bus_op);
					break;
				default: break;
			};

	};
	
};



enum SYS_ADDRESS_TYPE { ATYPE_NONE=0x1, ATYPE_REG=0x2, ATYPE_ROM=0x3, ATYPE_RAM=0x4, ATYPE_IO=0x5 };
typedef nibble_t SYS_addr_type_t;

typedef byte_t BPN_slot_num_t;

typedef longword_t CPU6_SYS_addr_t;
typedef word_t SYS_MMIO_addr_t;


typedef struct SYS_card_t {
	char *name;
	void *func;
	BPN_slot_num_t slot;
	byte_t (*bus_handler)(struct SYS_card_t *, BUS_CPU6_t *);
} SYS_card_t;

typedef byte_t (*bus_handler_t)(SYS_card_t *, BUS_CPU6_t *);

typedef struct BPN_slot_t {
	char *card_name;
	BPN_slot_num_t slot_num;
	bus_handler_t bus_handler;
} BPN_slot_t;

#define NUM_BPN_SLOTS 13

BPN_slot_t BPN_slots[NUM_BPN_SLOTS]; 

void BPN_init(BPN_slot_t slots[]) {
	for(int s=0; s < NUM_BPN_SLOTS ; s++ ) {
		slots[s].slot_num=s;	
		slots[s].card_name=0;	
		slots[s].bus_handler=0;
	}
}


typedef struct MUX_card_t {
	SYS_card_t *card;
	SYS_MMIO_addr_t IO_base;
} MUX_card_t;

void MUX_init(SYS_card_t *card, BPN_slot_t *BPN_slots[], BPN_slot_num_t slot, byte_t IO_base_dipswitch) {
	MUX_card_t *mux;
	mux= (MUX_card_t *)card->func;
	card->slot=slot;
	BPN_slots[slot]->card_name=card->name;
	BPN_slots[slot]->bus_handler=card->bus_handler;
	mux->IO_base= 0xf000 | ((IO_base_dipswitch & 0xf0)<<4);
}

byte_t MUX_bus_handler(SYS_card_t *card, SYS_BUS_t *bus) {
	MUX_card_t *mux;
	mux= (MUX_card_t *)card->func;
	byte_t ioff;
	if( ADDRESS_IN_RANGE(BUS_ADDRESS_MMIO(bus), mux->IO_base,0x000f) ) {
		ioff=ADDRESS_OFFSET(BUS_ADDRESS_MMIO(bus), mux->IO_base);
	} else {
		return(0);
	}

	
}


MUX_card_t MUX_card_0 = {
	.IO_base=0xf200
};

SYS_card_t SYS_MUX_card_0 = {
	.name="MUX Card 0",
	.func=&MUX_card_0,
	.slot=0xff,
	.bus_handler=&MUX_bus_handler
};



int main(int argc, char **argv) {
	CPU6_machine_t m;
	CPU6_eval_op(&m,0x01);
	return(0);
};
