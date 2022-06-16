#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*
 * In fact it turns out that what we have here is not a real
 * Centurion filesystem, but something like a tar archive. The real
 * filesystem, as understood from WIPL, has a different structure.
 * One directory entry occupies exactly 16 bytes; the first entry is
 * skipped when searching a file, presumably it is a volume header. Nothing
 * is known about the volume header's actual structure except that word at
 * offset 14 points to the first sector, where filesystem metadata (let's call
 * it disk map) is stored. The rest of entries have the following format:
 * struct DirEntry {
 *     char     name[10];   // File name, exactly 10 chars, padded up with spaces
 *     uint8_t  map_entry;  // Index of the first entry in the disk map sector, denoted below
 *     uint16_t map_sector; // Disk map sector, where the file begins (relative to disk map start)
 *     uint8_t  file_type;  // The bootloader checks for file_type & 0x0f == 4
 *     uint16_t unknown;
 * };
 * The directory terminates with first two characters of name containing bytes 0x84 0x0d
 * The disk map consists of 3-byte entries:
 * struct DiskMapEntry {
 *     uint8_t  entry_type;
 *     uint16_t payload;
 * };
 * "payload" has different meaning depending on entry_type:
 * entry_type > 0 - payload specifies starting sector (absolute) of the data cluster
 * entry_type < 0 - ~entry_type is a number of the next disk map sector (relative to disk map start),
 *                  and payload's LSB specifies an index of the starting DiskMapEntry in that sector
 * The first two map entries of a file constitute a header. We know nothing of the first entry,
 * payload of the second entry yields file cluster size by doing 1 << entry.payload
 * 
 * The structure below appears to be a combo of a volume header and DirEntry. We may presume
 * that the first 16 bytes are the same as on a bootable volume; it's logical to assume that a
 * volume has a name. Then we have a 0x848d terminator, which effectively makes the volume
 * containing zero files from WIPL's point of view. The data, following the terminator, effectively
 * repeats the original DirEntry the file was backed up from, at least in the image we have
 * dumping them reveals very consistent values. file_type also matches what's expected; @LOAD
 * file that we have in the archive has a type value of 0x14, which matches WIPL's expectations.
 * We may also speculate that the unknown field is also preserved, but we can't tell anything
 * about its actual meaning.
 * 
 * So, it appears that each file in the archive is contained inside its own "mini-volume",
 * and the archive is a chain of these "mini-volumes". The data appear contiguous, the disk
 * map is likely not present because map_start_sector in our image contains a value of 0x5779,
 * which does not represent a valid sector number (unless the archive spans accross several
 * Hawk removable platters, each of which constitutes a separate physical unit)
 */
typedef struct cent_dirent_t {
	char volname[10];
	uint8_t b0a_0d[4];
	uint8_t map_start_sector[2];
	uint8_t filemark[2];         /* Contains 0x848d */
	char filename[10];           /* VolumeEntry, shifted by 2 bytes */
	uint8_t map_entry;
	uint8_t map_sector[2];
	uint8_t filetype;
	uint8_t b20_2d[13];
	uint8_t next_sector[2];
	uint8_t b2f;
	char diskname[10];
	char seqnum[2];
	uint8_t b3d_3f[4];
} cent_dirent_t;

uint32_t sector_size = 400;

static uint16_t read_be16(const uint8_t *value) {
    return (value[0] << 8) | value[1];
}

void strip0x80(char * str, uint8_t n) {
	for(int i=0; i<n; i++) {
		*(str+i) &= 0x7f;
	}
}

int main(int argc, char **argv) {
	FILE *f;
	off_t pos;
	cent_dirent_t dirent,*de;
	uint32_t nsec = 16;
	uint32_t tsec, n;
	char *fn=argv[argc-1];

	de=&dirent;

	if( !( f= fopen(fn, "rb") ) ) {
		fprintf(stderr,"Failed to open '%s'\n",fn);
		perror("fopen:");
		return(-1);
	}

        for (int i = 1; i < argc - 1; i++) {
	    if (!strcmp(argv[i], "--ccdp")) {
	        printf("Using 512 bytes per block (CCDP format)\n");
	        sector_size = 512;
	    } else {
	        nsec=strtol(argv[i],NULL,0);
	    }
	}

	printf("Sector   Volume     File       Disk       Type MapStart MapSector MapEntry\n");

	n=0;
	do {
		tsec=nsec;
		pos=tsec * sector_size;
		if(fseeko(f,pos,SEEK_SET)) {
			fprintf(stderr,"Seek to 0x%lx failed\n",pos);
			perror("fseek:");
			fclose(f);
			return(-1);
		}
		if(!fread(de,sizeof(*de),1,f)) {
			fprintf(stderr,"Read dirent at 0x%lx failed\n",pos);
			perror("fread:");
			fclose(f);
			return(-1);
		}
		n++;
		nsec = read_be16(de->next_sector);
		strip0x80(de->volname,10);
		strip0x80(de->filename,10);
		strip0x80(de->diskname,10);
		strip0x80(de->seqnum,2);

		printf("0x%06x %.10s %.10s %.10s 0x%02x 0x%06x 0x%06x  %d\n", tsec, de->volname, de->filename, de->diskname,
		       de->filetype, read_be16(de->map_start_sector), read_be16(de->map_sector), de->map_entry);

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
