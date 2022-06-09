#include <stdint.h>
#include <stdio.h>
#include "DSK_hawk_format.h"
int main(int argc, char **argv) {
	FILE *fin=stdin, *fout=stdout;
	ssize_t o=0;
	char c;
	DSK_hawk_sector_address_t a;
	 
	while(1) {
		c=fgetc(fin);
		if(feof(fin)) { break;}
		if( o % HAWK_BYTES_PER_SECTOR == 0) {
			a.sector=(o/HAWK_BYTES_PER_SECTOR);
			fprintf(fout,"\nCHS: 0x%03x,%01x,%01x  (Sector: 0x%04x)\n",a.sector,a.C,a.H,a.S);
		}
		if(o % 0x10 == 0 ) { fprintf(fout,"+0x%02x:  ",(uint8_t)(o%10)); }
		fprintf(fout,"%02x%s",(uint8_t)c, (o%0x10==0xf)? "\n":" ");
		o++;
	}
	return(0);

}
