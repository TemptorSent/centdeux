#include "common-cents.h"
#include <stdio.h>

int main() {
	char str[]=CENT_HELLO;
	
	printf("str: '%s'\n", str);
	cents_nadd_0x80(str,9);
	printf("str: '%s'\n", str);
	cents_nstrip_0x80(str,9);
	printf("str: '%s'\n", str);
}
