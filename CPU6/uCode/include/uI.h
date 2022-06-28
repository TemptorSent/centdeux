#include "types-common.h"
#include "ALU_am2901_core.h"

typedef mouthful_t uIW_t;
typedef elevenbit_t uADDR_t;

typedef struct uI_dets_t {
	struct {
		bit_t begin;
		bit_t end;
	} loop;

	struct {
		char jsr;
	} sub;
	
	struct {
		bit_t CASE_;
		bit_t JSR_Di_;
	} cond;

	struct {
		uADDR_t prev, next;
		bit_t know_prev, know_next;
	} addr;

} uI_dets_t;


typedef struct uI_t {
	uADDR_t uADDR;
	uIW_t uIW;
	uI_dets_t dets;
	am2901_inst_t ALU_inst;
} uI_t;

void uI_decode(uI_t *uI);
