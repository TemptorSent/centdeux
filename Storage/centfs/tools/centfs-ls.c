#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct cent_dirent_t {
	char volname[10];
	uint8_t b0a_0f[6];
	uint8_t filetype[2];
	char filename[10];
	uint8_t b1c_b1f[4];
	uint8_t b20_2d[13];
	uint8_t next_sector[2];
	uint8_t b2f;
	char diskname[10];
	char volnum[2];
	uint8_t b3d_3f[4];
} cent_dirent_t;

#define SECSIZE 0x190

void strip0x80(char * str, uint8_t n) {
	for(int i=0; i<n; i++) {
		*(str+i) &= 0x7f;
	}
}

int main(int argc, char **argv) {
	FILE *f;
	fpos_t pos;
	cent_dirent_t dirent,*de;
	uint32_t tsec, nsec, n;
	char *fn=argv[argc-1];

	de=&dirent;

	if( !( f= fopen(fn, "rb") ) ) {
		fprintf(stderr,"Failed to open '%s'\n",fn);
		perror("fopen:");
		return(-1);
	}

	if(argc > 2) { nsec=strtol(argv[1],NULL,0); }
	else { nsec=0x10; }

	n=0;
	do {
		tsec=nsec;
		pos=tsec*SECSIZE;
		if(fsetpos(f,&pos)) {
			fprintf(stderr,"Seek to 0x%llx failed\n",pos);
			perror("fseek:");
			fclose(f);
			return(-1);
		}
		if(!fread(de,sizeof(*de),1,f)) {
			fprintf(stderr,"Read dirent at 0x%llx failed\n",pos);
			perror("fread:");
			fclose(f);
			return(-1);
		}
		n++;
		printf("ns0: 0x%2x  ns1: 0x%2x\n", de->next_sector[0], de->next_sector[1]);
		nsec=(de->next_sector[0]<<8) | (de->next_sector[1]);
		strip0x80(de->volname,10);
		strip0x80(de->filename,10);
		strip0x80(de->diskname,10);

		printf("Sector: 0x%06llx\n",pos/0x190);
		printf("Volume Name: '%.10s'\n", de->volname);
		printf("File Name: '%.10s'\n", de->filename);
		printf("Disk Name: '%.10s'\n", de->diskname);
		printf("Next File Start Sector: 0x%0x\n",nsec);

		if(!nsec) {
			printf("\nFound %u entries.\n",n);
			fclose(f);
			return(0);
		}

		if(nsec == 0xffff ) {
			printf("\nFound %u entries on volume, older entries may exist.\n",n);
			fclose(f);
			return(0);
		}






		
	} while( !feof(f) );
	printf("\nEOF reached after %u entries.\n",n);
	fclose(f);
	return(0);
	
}
