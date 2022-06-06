#include <stdio.h>
/* Read bytes from stdin, strip the high bit of each byte, and write to stdout */
int main(int argc, char **argv) {
	char c;
	FILE *fin=stdin;
	while(1) {
		c=fgetc(fin);
		if(feof(fin)) { break;}
		c &= 0x7f;
		printf("%c",c);
	}
	return(0);
}
