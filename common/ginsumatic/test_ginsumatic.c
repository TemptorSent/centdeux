#include "ginsumatic.h"
int main() {
	char blendtec[8]="\x00\x01\x02\x03\x07\x06\x05\x04";
	uint8_t in=0x55;
	uint8_t *s[]={&in};
	byte_ptr_list_t t={&in};
	uint8_t out;
	uint16_t out2;
	bitblender_t blend = {{&out}, 8,blendtec,&t};
	uint8_t inA=0x05, inB=0x0a, inC=0x0f;
	byte_ptr_list_t seq_output_list={&inA, &inB, &inC};
	char *seq_output_order="\x00\x01\x02\x03\xff\xff\xff\xff\x04\x05\x06\x07\xff\xff\xff\xff\x08\x09\x0a\x0b";
	bitblender_t seq_output_blender={{.w=&out2},11,seq_output_order,&seq_output_list};
	bitblend(&blend);
	printf("blending %02x -> %02x\n", in, out);
	
	bitblend(&seq_output_blender);
	printf("blending %02x %02x %02x -> %04x\n", inA,inB,inC, out2);
	
	
	printf("blending %02x -> %02x\n", in, bitblend_8(8,blendtec,s));
	return(0);
}
