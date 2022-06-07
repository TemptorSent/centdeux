#include <stdio.h>

int main(int argc, char **argv) {
	FILE *fin=stdin, *fout=stdout;
	ssize_t o=0, cblk=0x190, fblk=0x200;
	char c;
	 
	while(1) {
		c=fgetc(fin);
		if(feof(fin)) { break;}
		if( o++ % fblk < cblk) { fputc(c,fout); }
	}
	return(0);

}
