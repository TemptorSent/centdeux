#include <stdio.h>

int main(int argc, char **argv) {
	FILE *fin=stdin, *fout=stdout;
	ssize_t o=0, cblk=0x190, fblk=0x200;
	char c;
	 
	while(1) {
		c=fgetc(fin);
		if(feof(fin)) { break;}
		if( o++ % fblk < cblk) { fputc(c,fout); }
		else if( c==0 && o % fblk == cblk+1 ) { fprintf(stderr, "Bad sector 0x%06lx\n",o/fblk); }
	}
	return(0);

}
