#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include "common-cents.h"
#include "centfs-disk-format.h"
#include "ccdp-dump-format.h"
#include "centfs.h"


#define BE_WORD(a) ( ( *((a)+0x0) << 8 ) | ( *((a)+0x1) << 0 ) )

#define BE_3BYTES(a) (  ( *((a)+0x0) << 16 ) | ( *((a)+0x1) << 8 ) | ( *((a)+0x2) << 0 ) )


typedef struct centfs_image_file_t {
	char *filename;
	FILE *file;
	centfs_sector_byte_offset_t sector_size;
} centfs_image_file_t;



int centfs_image_read_sector(centfs_device_t *dev, centfs_sector_t *sec) {
	centfs_image_file_t *img;
	off_t pos;
	size_t res;
	uint8_t extra=0;

	img= (centfs_image_file_t *)(dev->internal);
	pos= sec->number * img->sector_size;

	if(fseeko(img->file,pos,SEEK_SET)) {
		fprintf(stderr,"Seek to sector 0x%04x failed\n",sec->number);
		perror("fseeko:");
		fclose(img->file);
		dev->state=CENTFS_DEV_IOSTATE_IOERROR;
		return(-1);
	}

	res=fread(&sec->data,1,sizeof(sec->data),img->file);
	if( res < sizeof(sec->data) ) {
		fprintf(stderr,"Read sector 0x%04x failed\n",sec->number);
		perror("fread:");
		fclose(img->file);
		dev->state=CENTFS_DEV_IOSTATE_IOERROR;
		return(-1);
	}

	if( img->sector_size > sizeof(sec->data) ) {
		res=fread(&extra,1,1,img->file);
		if( !res ) {
			fprintf(stderr,"Read extra bytes sector for 0x%04x failed\n", sec->number);
			perror("fread:");
			fclose(img->file);
			dev->state=CENTFS_DEV_IOSTATE_IOERROR;
			return(-1);
		} else if(!extra) {
			sec->state=CENTFS_SECTOR_IOSTATE_READ_ERROR;
			return(0);
		}
	}

	sec->state=CENTFS_SECTOR_IOSTATE_READ_VALID;
	return(0);
}

int centfs_image_detect_sector_size(centfs_device_t *dev) {
	centfs_image_file_t *img;
	int res;
	uint8_t c=0, p=0;
	uint16_t i=0, t=0, ff=0;

	img= (centfs_image_file_t *)(dev->internal);

	/* Seek to beginning of track 0, sector 0 */
	if(fseeko(img->file,0,SEEK_SET)) {
		fprintf(stderr,"Seek to sector 0 failed\n");
		perror("fseeko:");
		fclose(img->file);
		dev->state=CENTFS_DEV_IOSTATE_IOERROR;
		return(-1);
	}

	/* Iterate over the first 0x401 bytes of the image and detect a run of 0xff bytes following the native sector end */
	for(i=0; i < 0x401; i++) {
		if( (res=fgetc(img->file)) == EOF ) {
			fprintf(stderr,"Reading image failed at byte 0x%06x\n",i);
			perror("fgetc:");
			fclose(img->file);
			dev->state=CENTFS_DEV_IOSTATE_IOERROR;
			return(-1);
		}

		c=(uint8_t)res;
		if( i < CENTFS_BYTES_PER_SECTOR ) {
			if( c!= p ) { t++; p=c; }
		}
		if( i >= CENTFS_BYTES_PER_SECTOR ) {
			if( c == 0xff ) { ff++; }
			else { break; }
		}	
	}


	if( t < 2 ) {
		fprintf(stderr, "WARNING: track 0, sector 0 of image appears to not contain valid data!\n");
	}

	if( i < 0x200 ) {
		if( i > CENTFS_BYTES_PER_SECTOR ) {
			fprintf(stderr, "Note: Got run of 0x%04x 0xff bytes following expected end of sector,", ff);
			fprintf(stderr, "but not enough to reach 0x200.\n");
			fprintf(stderr, "Asuming native sector size of 0x%04x bytes per sector.\n", CENTFS_BYTES_PER_SECTOR);
		} else {
			fprintf(stderr, "Info: Detected sector size of 0x%04x - override manually if needed\n", i);
		}

	       	return(CENTFS_BYTES_PER_SECTOR);
	} else if( i % 0x100 ) {
		fprintf(stderr, "Note: Got run of 0x%04x 0xff bytes following expected end of sector,", ff);
		fprintf(stderr, "which results in a sector size 0x%04x that is not an even multiple of 0x100\n", i);
		i= (i % 0x100);
		fprintf(stderr, "Assuming sector size of 0x%04x - override manually if needed\n",i);
	} else if( i == 0x400 ) { 
		fprintf(stderr, "Note: Got run of 0x%04x 0xff bytes following expected end of sector,", ff);
		fprintf(stderr, "which would exceed 0x400 bytes total sector length.");
		i=0x400;
		fprintf(stderr, "Assuming sector size of 0x%04x - override manually if needed\n", i);
	} else {
		fprintf(stderr, "Info: Detected sector size of 0x%04x - override manually if needed\n", i);
	}

	return(i);		

};

int centfs_image_device_open(centfs_device_t *dev) {
	centfs_image_file_t *img;
	img= (centfs_image_file_t *)(dev->internal);
	if( !( img->file= fopen(img->filename, "rb") ) ) {
		fprintf(stderr,"Failed to open '%s'\n",img->filename);
		perror("fopen:");
		dev->state=CENTFS_DEV_IOSTATE_IOERROR;
		return(-1);
	}
	dev->state=CENTFS_DEV_IOSTATE_OPEN;
	return(0);
}
int centfs_image_device_close(centfs_device_t *dev) {
	centfs_image_file_t *img;
	img= (centfs_image_file_t *)(dev->internal);
	fclose(img->file);
	dev->state=CENTFS_DEV_IOSTATE_CLOSED;
	return(0);
}

int centfs_image_device_init(
	centfs_device_t *dev,
	centfs_image_file_t *img,
	char *filename,
	centfs_sector_byte_offset_t sector_size
) {
	int res;
	dev->internal=(void *)img;
	dev->device_open= &centfs_image_device_open;
	dev->device_close= &centfs_image_device_close;
	dev->read_sector= &centfs_image_read_sector;
	img->filename= filename;
	res=centfs_image_device_open(dev);
	if(res < 0) { return(res); }
	res= sector_size? sector_size : centfs_image_detect_sector_size(dev);
	if( res < 0 ) {
		img->sector_size=0;
		return(res);
	} else { 
		img->sector_size=res;
	}
	return(0);
}

void centfs_print_dr_header(centfs_dr_header_t *dr0) {
	char fn[sizeof(dr0->volume_name)];
	cents_ncopy_strip_0x80(fn,dr0->volume_name,sizeof(dr0->volume_name));
	
	printf("%.*s",(int)sizeof(fn), fn);
	printf("\t0x%04x",BE_WORD(dr0->last_reorg_date));
	printf("\t0x%02x",dr0->last_reorg_errors);
	printf("\t0x%06x",BE_3BYTES(dr0->alist_sector_start));
	printf("\n");
	return;
}


void centfs_print_dr_file(centfs_dr_file_t *drf) {
	char fn[sizeof(drf->filename)];
	cents_ncopy_strip_0x80(fn,drf->filename,sizeof(drf->filename));
	
	printf("%.*s",(int)sizeof(fn), fn);
	printf("\t0x%02x",drf->alist_entry_number);
	printf("\t0x%04x",BE_WORD(drf->alist_sector_offset));
	printf("\t0x%02x",drf->filetype);
	printf("\t0x%04x",BE_WORD(drf->date));

	return;
}

int centfs_print_alist_attr(centfs_alist_attr_t *attr) {
	printf("\t0x%04x",BE_WORD(attr->filesize));
	printf("\t0x%02x",BE_WORD(attr->file_begin_pointer));
	printf("\t0x%02x",attr->filesize_increment);
	printf("\t0x%02x",attr->fileclass);
	return(0);
}

int32_t centfs_read_alist_dalent( centfs_dirent_t *dirent, centfs_sector_byte_t *next_ptr) {
	centfs_sector_t alsec;
	centfs_sector_addr_t *dals;

	centfs_sector_byte_t dummy_next={.sector=0, .byte=0 };
	centfs_sector_byte_t *next;

	if(!next_ptr) { next=&dummy_next; }
	else { next=next_ptr; };

	if(!(next->sector || next->byte)) { 
		next->sector = dirent->dal_ptr.sector;
		next->byte = dirent->dal_ptr.byte;
       	} 

centfs_read_alist_dalent_readnext:
	alsec.number=next->sector;
	(dirent->dev->read_sector)(dirent->dev,&alsec);

	dals=(centfs_sector_addr_t *)(alsec.data + next->byte);
	if((*dals)[0] & 0x80 ) {
		if((*dals)[1] == 0xff) { return(-1); }
		next->sector = dirent->base_sector + (uint16_t)( ~((uint16_t)(BE_WORD(*dals))) );
		next->byte= 3 * (*dals)[2];
		goto centfs_read_alist_dalent_readnext;

	}
	next->byte += 3;
	return(BE_3BYTES(*(dals)) + (BE_3BYTES(dirent->header.alist_sector_start)? 0 : dirent->base_sector));
}

int centfs_print_alist_dal(centfs_dirent_t *dirent) {
	centfs_sector_byte_t next_dalent = {.sector=0, .byte=0};
	int32_t dale;

	for(int i=0; !((dale = centfs_read_alist_dalent(dirent,&next_dalent)) < 0); i++) {
		printf("%c0x%06x", i? ' ':'\t', dale);
	}

	return(0);
}

int centfs_read_alist_attr(centfs_dirent_t *dirent) {
	centfs_alist_attr_t *attr;
	centfs_sector_t alsec;
	alsec.number=dirent->attr_ptr.sector;

	(dirent->dev->read_sector)(dirent->dev,&alsec);
	attr= (centfs_alist_attr_t*)(alsec.data + dirent->attr_ptr.byte);
	memcpy(&(dirent->attr), attr, sizeof(*attr));


	return(0);
}

int centfs_read_dirent_header(centfs_dirent_t *dirent) {
	centfs_sector_t drsec;
	centfs_dr_header_t *dr0;

	drsec.number=dirent->base_sector;

	if( ( (dirent->dev->read_sector)(dirent->dev,&drsec) ) != 0 ) {
		return(-1);
	}

	dr0=(centfs_dr_header_t *)(drsec.data);

	memcpy(&(dirent->header), dr0, sizeof(*dr0));
	
	return(0);
}

int centfs_read_dirent_file(centfs_dirent_t *dirent) {
	centfs_sector_t drsec;
	centfs_dr_file_t *dr;
	centfs_sector_byte_offset_t byte_offset;
	centfs_sector_number_t sector_offset;
	
	sector_offset= dirent->file_idx / CENTFS_DRS_PER_SECTOR;
	byte_offset= (dirent->file_idx % CENTFS_DRS_PER_SECTOR) * CENTFS_DR_LENGTH;

	drsec.number= dirent->base_sector + sector_offset;

	if( ( (dirent->dev->read_sector)(dirent->dev,&drsec) ) != 0 ) {
		return(-1);
	}

	dr=(centfs_dr_file_t *)((drsec.data) + byte_offset);
	if( ! dr->filename[0] ) {
		return(0);
	}

	memcpy(&(dirent->file), dr, sizeof(*dr));


	dirent->attr_ptr.sector= ( BE_WORD(dirent->file.alist_sector_offset) +
		( BE_3BYTES(dirent->header.alist_sector_start)?
			BE_3BYTES(dirent->header.alist_sector_start)
			: dirent->base_sector
		)
	);

	dirent->attr_ptr.byte=dr->alist_entry_number * CENTFS_ALIST_DALENT_LENGTH;
	
	centfs_read_alist_attr(dirent);


	dirent->dal_ptr.sector=dirent->attr_ptr.sector;
	dirent->dal_ptr.byte=dirent->attr_ptr.byte + CENTFS_ALIST_ATTR_LENGTH;

	return(dirent->file_idx);
}

int centfs_dir_list( centfs_dirent_t *dirent) {
	centfs_sector_t drsec;
	centfs_dr_header_t *dr0;
	centfs_dr_file_t *dr;
	centfs_dirent_t subdirent;
	centfs_sector_number_t alist_sector_start, alist_sec;
	int res=0;
	centfs_sector_byte_offset_t offset;
	void *ent;

	drsec.number=dirent->base_sector;

	res=centfs_read_dirent_header(dirent);
	if(res < 0 ) { return(res); }

	if(!dirent->parent_dirent) { centfs_print_dr_header(&dirent->header); }

	for (dirent->file_idx=1; (res=centfs_read_dirent_file(dirent))>0; dirent->file_idx++) {
		centfs_read_alist_attr(dirent);

		if(dirent->parent_dirent) { printf("  "); }
		centfs_print_dr_file(&dirent->file);
		centfs_print_alist_attr(&dirent->attr);
		printf("\n");

		if( (dirent->file.filetype & 0x0f ) == 0x5 ) {
			subdirent.parent_dirent=dirent;
			subdirent.dev=dirent->dev;
			subdirent.base_sector=centfs_read_alist_dalent(dirent,(void *)0);
			centfs_dir_list(&subdirent);
		}


		centfs_print_alist_dal(dirent);
		printf("\n");


	}
	if( res < 1 ) { return(res); }
	return(dirent->file_idx);
}

int centfs_fnlen(char *fn) {
	uint8_t c;
	for(size_t i=strnlen(fn,CENTFS_FILENAME_BYTES); i > 0; i--) {
		c=(uint8_t)*(fn+(i-1));
		if( c != (uint8_t)CENT_CHAR(' ') )  {
			return(i);
		}
	}
	return(0);
}

char *centfs_fnsep(char *fn) {
	return( (fn==NULL)?  NULL: (char *)memchr(fn, (uint8_t)CENT_CHAR('.'), strnlen(fn,CENTFS_FILENAME_BYTES)) );
}

char *centfs_fnsplit(char *fn) {
	char *sep;
	sep= centfs_fnsep(fn);
	return( sep? (sep+1) : NULL );
}

int centfs_fnmatch(char *fn, char *match) {
	size_t fnl;
	char *msep;
	size_t ml;
	size_t fi=0;
	char mt[22];

	if( match == NULL) { return(0); }
	fnl= centfs_fnlen(fn);
	msep= centfs_fnsep(match);
	ml= msep? (msep - match) : centfs_fnlen(match);

	for( size_t mi=0; mi < ml ; mi++ ) {
		uint8_t m= (uint8_t)*(match+mi);
		uint8_t n= (uint8_t)(((mi+1) == ml)? '\0' : *(match+mi+1));
		uint8_t f= (uint8_t)*(fn+fi);

		if( m == (uint8_t)CENT_CHAR('*') ) {
			if( n == (uint8_t)CENT_CHAR('*') ) {
				continue;
			} else if ( n == (uint8_t)'\0' ) {
				return(0);
			} else {
				while(fi < fnl) {
					f= (uint8_t)*(fn+fi);
					if(f == n) {
						goto centfs_fnmatch_next;
					}
					fi++;
				}
				return(-1);
			}
		} else if ( f != m ) {
			return(-1);
		}
		fi++;
centfs_fnmatch_next:
	}
	if (fi < fnl ) { return(-1); }
	else { return(0); }
}

int centfs_dir_find( centfs_dirent_t *dirent, char * match) {
	centfs_sector_t drsec;
	centfs_dr_header_t *dr0;
	centfs_dr_file_t *dr;
	centfs_dirent_t subdirent;
	centfs_sector_number_t alist_sector_start, alist_sec;
	int res=0;
	int found=0;
	centfs_sector_byte_offset_t offset;
	void *ent;

	drsec.number=dirent->base_sector;

	res=centfs_read_dirent_header(dirent);
	if(res < 0 ) { return(res); }

	if(!dirent->parent_dirent) { centfs_print_dr_header(&dirent->header); }

	for (dirent->file_idx=1; (res=centfs_read_dirent_file(dirent))>0; dirent->file_idx++) {
		if( !centfs_fnmatch(dirent->file.filename, match) ) {
			if( centfs_fnsep(match)  ) {
				if( (dirent->file.filetype & 0x0f) == CENTFS_FILETYPE_LIB ) {
					//printf("Found LIB! %u\n", dirent->file_idx);
				} else { continue; }
			} else {
				//printf("Found! %u\n", dirent->file_idx);
				found++;
			}
		} else { continue; }

		centfs_read_alist_attr(dirent);

		if(dirent->parent_dirent) { printf("  "); }
		centfs_print_dr_file(&dirent->file);
		centfs_print_alist_attr(&dirent->attr);
		printf("\n");

		/* "library file" is the term used for a sub directory */
		if( (dirent->file.filetype & 0x0f ) == CENTFS_FILETYPE_LIB ) {
			subdirent.parent_dirent=dirent;
			subdirent.dev=dirent->dev;
			subdirent.base_sector=centfs_read_alist_dalent(dirent,(void *)0);
			centfs_dir_find(&subdirent,centfs_fnsplit(match));
		}


		centfs_print_alist_dal(dirent);
		printf("\n");


	}
	if( res < 1 ) { return(res); }
	return(found);
}

void usage(char *progname) {
	printf("Usage:");
	printf(" %s [IMAGE-OPTION]... IMAGE-FILE SUBCMD [SUBCMD-OPTION]... [FILENAME]\n", progname);
	printf("List or extract file from a Centurion filesystem image.\n\n");
	printf("IMAGE-OPTIONS:\n");
	printf("  -b, --base-dir-sector        sector address of beginning of directory\n");
	printf("  -s, --sector-size            image format sector size in bytes\n");
	printf("  -C, --ccdp                   use settings for reading CCDP dumped images\n");
	printf("SUBCMD-LIST-OPTIONS:\n");
	printf("SUBCMD-EXTRACT-OPTIONS:\n");
	exit(-1);
}


int main(int argc, char **argv) {
	centfs_image_file_t img;
	centfs_dr_header_t *dr0;
	centfs_dr_file_t *dr;
	centfs_sector_t drsec,mapsec;
	centfs_device_t dev;
	centfs_sector_number_t alist_sector_start;
	centfs_dirent_t dirent;
	int found=0;
	char *match;
	char mt[23];
	char *mtp;	
	char dirname[sizeof(dr->filename)];
	char *imagefile;
	int argr, argn;
	int opt, optidx;
	int res;
	char *optstr;
	const struct option *long_opts;
	char **argp;

	centfs_sector_number_t base_dir_sector=0x000010;
	centfs_sector_byte_offset_t sector_size=0;

	mtp=mt;

	uint32_t tsec, nsec, n;

	enum SUBCMDS { SUBCMD_NONE=0, SUBCMD_LIST, SUBCMD_EXTRACT };
	struct subcmd_ent {
		char *name;
		enum SUBCMDS subcmd;
	};
		
	const struct subcmd_ent subcmd_list[] = {
		{"list", SUBCMD_LIST },
		{"ls", SUBCMD_LIST },
		{"extract", SUBCMD_EXTRACT },
		{"x", SUBCMD_EXTRACT },
		{0,0}
	};
	enum SUBCMDS subcmd=SUBCMD_NONE;


	const struct option image_long_opts[] = {
		{"help", no_argument, NULL, 'h' },
		{"sector-size", required_argument, NULL, 's' },
		{"device-type", required_argument, NULL, 't' },
		{"base-dir-sector", required_argument, NULL, 'b' },
		{"ccdp", no_argument, NULL, 'C' },
		{0,0,0,0}
	};
	char *image_optstr="+:s:t:C";

	const struct option list_long_opts[] = {
		{"all", no_argument, NULL, 'a'},
		{"recursive", no_argument, NULL, 'R' },
		{"long", no_argument, NULL, 'l' },
		{"directory", no_argument, NULL, 'd'},
		{"sector-index", no_argument, NULL, 'i'},
		{0,0,0,0}
	};

	char *list_optstr= "+:aRldi";

	const struct option extract_long_opts[] = {
		{"all", no_argument, NULL, 'a'},
		{"recursive", no_argument, NULL, 'R'},
		{0,0,0,0}
	};

	char *extract_optstr= "+:arR";


	while(1) {
		opt= getopt_long(argc,argv,image_optstr,image_long_opts,&optidx);
		if( opt == -1 ) { break; }
		switch(opt) {
			case 'h': usage(argv[0]); break;
			case 's': sector_size=strtol(optarg,NULL,0); break;
			case 'b': base_dir_sector=strtol(optarg,NULL,0); break;
			case 't': printf("device-type option not yet implemented\n"); break;
			case 'C': sector_size=CCDP_DUMP_BYTES_PER_SECTOR; break;
		}
	}
	
	argr=argc-optind;
	argn=optind;

	if(argr < 1 ) {
		printf("Requires filename as argument.\n");
		usage(argv[0]);
	} else if( (res= centfs_image_device_init(&dev,&img,argv[argn],sector_size)) ) {
		printf("Could not initilize image device.\n");
		return(-ENODEV);
	}
	argn++;
	argr--;

	if(argr) {
		char *n;
		for(int i=0; (n=subcmd_list[i].name); i++) {
			if( ! strncasecmp(n, argv[argn], strnlen(argv[argn], strnlen(n,10))) ) {
				subcmd= subcmd_list[i].subcmd;
			}
		}
		if( subcmd == SUBCMD_NONE ) {
			usage(argv[0]);
		}

	} else {
		usage(argv[0]);
	}




	switch(subcmd) {
		case SUBCMD_LIST: optstr=list_optstr; long_opts=list_long_opts; break;
		case SUBCMD_EXTRACT: optstr=extract_optstr; long_opts=extract_long_opts; break;
		default: printf("Bad subcommand\n"); usage(argv[0]); break;
	}

	/* Reset our getopt vars for another run */
	optind=0;
	optidx=0;
	argp=&(argv[argn]);
	while(1) {
		opt= getopt_long(argr,argp,optstr,long_opts,&optidx);
		if( opt == -1 ) { break; }
		switch(subcmd) {
			case SUBCMD_LIST:
				switch(opt) {
					case 'a':
						printf("list all\n");
						break;
					case 'R':
						break;
					case 'l':
						break;
					case 'i':
						break;
				}
				break;
			case SUBCMD_EXTRACT:
				switch(opt) {
					case 'a':
						break;
					case 'R':
					case 'r':
						break;

				}
				break;
			default:
				printf("Bad subcommand\n");
				usage(argv[0]);
				break;
		}
	}


	argr=argr-optind;
	argn=optind;

	if(argr) {
		match=argp[argn];
	} 





	dirent.dev=&dev;
	dirent.base_sector=base_dir_sector;
	dirent.parent_dirent=0;

	printf("Searching for '%.10s'\n", match);
	if(match) { cents_nadd_0x80(match,strnlen(match,10)); }
	found=centfs_dir_find(&dirent,match);

	return(0);
	
}
