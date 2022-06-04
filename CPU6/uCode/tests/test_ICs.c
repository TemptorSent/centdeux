#include <stdio.h>
#include "../include/cpu6_ICs.h"
int main(int argc, char **argv) {
	U_out_t *o;
	U_in_t *in;
	U_en_t *en, *clr;
	U_sel_t *sel;
	//printf("U%c%u%s: %s\n",UD2A.col, UD2A.row, UD2A.unit?&UD2A.unit:"", UD2A.name);
	//printf("%s\n",IC_get_out(UD2A,3).LO.d);
	//for(int i=0; (o=IC_get_out(UD2A,i)).LO.d; i++) {
	//	printf("%s\n",o.LO.d);
	//}

	U_t *ic;

	for(int i=0; (ic=ICs[i])!=0;i++) {
		printf("U%c%u%s: %s\n",ic->col, ic->row, (ic->unit? &(ic->unit) : "") ,ic->name);
		if(!ic->en) { goto skip_en;} 
		printf("\tEnables:\n");
		for(int j=0; j<8 ;j++) {
			en=&(*ic->en)[j];
			if(!en->src && !en->d && !en->LO.d && !en->HI.d && !en->sig ) { goto skip_en;}
			printf("\t\t");
			if(en->src == SRC_uIW) { printf("uIW<0%02o>  ",en->uIW.bit); }
			printf("%s\n",en->LO.d?en->LO.d:en->HI.d?en->HI.d:en->d?en->d:"");
		}
		skip_en:
		if(!ic->sel) { goto skip_sel;} 
		printf("\tAddress Selects:\n");
		for(int j=0; j<8 ;j++) {
			sel=&(*ic->sel)[j];
			if(!sel->src && !sel->d && !sel->LO.d && !sel->HI.d && !sel->sig ) { goto skip_sel;}
			printf("\t\t");
			if(sel->src == SRC_uIW) { printf("uIW<0%02o>  ",sel->uIW.bit); }
			printf("%s\n",sel->LO.d?sel->LO.d:sel->HI.d?sel->HI.d:sel->d?sel->d:"");
		}
		skip_sel:
		if(!ic->clr) { goto skip_clr;} 
		printf("\tClears/Resets:\n");
		for(int j=0; j<8 ;j++) {
			clr=&(*ic->clr)[j];
			if(!clr->src && !clr->d && !clr->LO.d && !clr->HI.d && !clr->sig ) { goto skip_clr;}
			printf("\t\t");
			if(clr->src == SRC_uIW) { printf("uIW<0%02o>  ",clr->uIW.bit); }
			printf("%s\n",clr->LO.d?clr->LO.d:clr->HI.d?clr->HI.d:clr->d?clr->d:"");
		}
		skip_clr:
		if(!ic->in) { goto skip_in;} 
		printf("\tInputs:\n");
		for(int j=0; j<8 ;j++) {
			in=&(*ic->in)[j];
			if(!in->src && !in->d && !in->LO.d && !in->HI.d && !in->sig ) { goto skip_in;}
			printf("\t\t");
			if(in->src == SRC_uIW) { printf("uIW<0%02o>  ",in->uIW.bit); }
			printf("%s\n",in->LO.d?in->LO.d:in->HI.d?in->HI.d:in->d?in->d:"");
		}
		skip_in:
		if(!ic->out) { goto skip_out;} 
		printf("\tOutputs:\n");
		for(int j=0; j<8 ;j++) {
			o=&(*ic->out)[j];
			if(!o->d && !o->LO.d && !o->HI.d && !o->sig ) { goto skip_out;}
			printf("\t\t%s\n",o->LO.d?o->LO.d:o->HI.d?o->HI.d:o->d?o->d:"");
		}
		skip_out:
	}
	return(0);
}
