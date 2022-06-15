#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include "DSK-hawk-format.h"

#define BYTES_PER_LINE 0x10

int main(int argc, char **argv) {
	FILE *fin=stdin, *fout=stdout;
	ssize_t o=0;
	char c, c7b;
	char txt[BYTES_PER_LINE+1];
	DSK_hawk_sector_address_t a;

	txt[BYTES_PER_LINE]=0;	

	while(1) {
		c=fgetc(fin);
		if(feof(fin)) { break;}
		if( o % HAWK_BYTES_PER_SECTOR == 0) {
			a.sector=(o/HAWK_BYTES_PER_SECTOR);
			fprintf(fout,"\nCHS: 0x%03x,%01x,%01x  (Sector: 0x%04x)\n",a.C,a.H,a.S,a.sector);
		}
		c7b=c&0x7f;
		txt[o % BYTES_PER_LINE]=(isprint(c7b)? c7b : '.');
		if(o % BYTES_PER_LINE == 0 ) { fprintf(fout,"+0x%03x:  ",(uint16_t)(o % HAWK_BYTES_PER_SECTOR)); }
		if( (o % BYTES_PER_LINE) == (BYTES_PER_LINE - 1) ) {
			fprintf(fout,"%02x\t%s\n",(uint8_t)c, txt );

		} else {
			fprintf(fout,"%02x  ",(uint8_t)c);
		}
		o++;
	}
	return(0);

}
