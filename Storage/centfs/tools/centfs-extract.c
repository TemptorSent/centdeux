#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
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

int centfs_dirent_init(centfs_dirent_t *dirent, centfs_device_t *dev, centfs_sector_number_t base_sector, centfs_dirent_t *parent){
	dirent->dev=dev;
	dirent->base_sector=base_sector;
	dirent->parent_dirent=parent;
	return(0);
}

void centfs_print_fn(char *fn) {
	char f[CENTFS_FILENAME_BYTES] = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' };
	cents_ncopy_strip_0x80(f,fn,CENTFS_FILENAME_BYTES);
	printf("%.*s",CENTFS_FILENAME_BYTES, f);
}

void centfs_print_filetype(uint8_t type) {
	type &= 0xf;
	if(type > CENTFS_FILETYPE_MAX) { return; }
	printf("%.3s", centfs_filetype_list[type][1] );
}

void centfs_print_datestamp(uint16_t ds) {
	struct tm tm;
	//if(!ds) { printf("0000-00-00"); return; }

	tm.tm_sec=0;
	tm.tm_min=0;
	tm.tm_hour=0;
	tm.tm_mon=0;
	tm.tm_mday=ds+1;
	tm.tm_year=0;
	tm.tm_wday=0;
	tm.tm_yday=0;
	tm.tm_isdst=-1;
	mktime(&tm);
	printf("%04u-%02u-%02u",(tm.tm_year + 1900), (tm.tm_mon + 1), (tm.tm_mday ) ); 

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
	//printf("\t0x%02x",drf->filetype);
	printf("\t");
	centfs_print_filetype(drf->filetype);
	//printf("\t0x%04x",BE_WORD(drf->date));
	printf("\t");
	centfs_print_datestamp(BE_WORD(drf->date));

	return;
}

int centfs_print_alist_attr(centfs_alist_attr_t *attr) {
	centfs_sector_number_t bc, bs, fs;
	int32_t fsb;

	fs= BE_WORD(attr->filesize) + 1;
	bs= (1 << attr->filesize_increment);
	bc= fs / bs;
	fsb= fs * CENTFS_BYTES_PER_SECTOR;

	printf("\t0x%04x sectors (0x%04x 0x%04x sector blocks) [0x%06x bytes]", fs, bc, bs, fsb);

	//printf("\t0x%04x",BE_WORD(attr->filesize));
	printf("\t0x%02x",BE_WORD(attr->file_begin_pointer));
	//printf("\t0x%02x",attr->filesize_increment);
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

int centfs_dirent_read_file(centfs_dirent_t *dirent) {
	centfs_sector_t fsec;
	centfs_sector_byte_t next_dalent = {.sector=0, .byte=0};
	int32_t dale;

	for(int i=0; !((dale = centfs_read_alist_dalent(dirent,&next_dalent)) < 0); i++) {
		for( fsec.number=dale; fsec.number < (dale + (1 << dirent->attr.filesize_increment)); fsec.number++ ) {
			(dirent->dev->read_sector)(dirent->dev,&fsec);
			fwrite(fsec.data, sizeof(fsec.data), 1, stdout);
		}

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

centfs_sector_t *centfs_FILE_seek_sector(centfs_FILE *F, centfs_sector_number_t file_sectnum) {
	centfs_sector_byte_t next_dalent = {.sector=0, .byte=0};
	int32_t dale;
	for(int i=0; !((dale = centfs_read_alist_dalent(F->dirent,&next_dalent)) < 0); i++) {
		for(int j=0; j < (1 << F->dirent->attr.filesize_increment); j++ ) {
			if( !(file_secnum-- > 0) ) {
				F->sector.number= dale + j;
				return(F->sector);
			}
		}
	}
	return(NULL);
}

centfs_sector_t *centfs_FILE_read_sector(centfs_FILE *F) {
	if( !(F->dirent->dev->read_sector)(F->dirent->dev,F->sector) ) {
		return(F->sector);
	} else {
		return(NULL);
	}
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
	return( sep? *(sep+1)? (sep+1)  : NULL : NULL );
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


typedef struct centfs_dirent_callback_t {
	int (*func)(centfs_dirent_t *, struct centfs_dirent_callback_t *);
	void *opts;
	int count;
} centfs_dirent_callback_t;

typedef int (*centfs_dirent_callback_func_t)( centfs_dirent_t *, centfs_dirent_callback_t * );

typedef struct centfs_dirent_callback_list_opts_t {
	uint8_t mode_long;
	uint8_t mode_all;
	uint8_t mode_sector_index;
} centfs_dirent_callback_list_opts_t;

int centfs_dirent_callback_list( centfs_dirent_t *dirent, centfs_dirent_callback_t *callback) {
       	centfs_dirent_callback_list_opts_t *opts;
	opts=(centfs_dirent_callback_list_opts_t *)callback->opts;

	if(dirent->file_idx==0 && !dirent->parent_dirent) { centfs_print_dr_header(&dirent->header); }
	if( !opts->mode_all && (dirent->file.filetype & 0x0f) == CENTFS_FILETYPE_DIR ) { return(-1); };
	if(dirent->parent_dirent) {
		if( callback->count++ == 0 ) {
			centfs_print_fn(dirent->parent_dirent->file.filename);
			printf("\n");
		}
		printf("  ");
	}
	centfs_print_dr_file(&dirent->file);
	if( opts->mode_long ) { centfs_print_alist_attr(&dirent->attr); }
	printf("\n");
	if( opts->mode_sector_index ) {
		centfs_print_alist_dal(dirent);
		printf("\n");
	}


	return(0);
}

typedef struct centfs_dirent_callback_extract_opts_t {
	uint8_t mode_ascii;
	uint8_t mode_all;
} centfs_dirent_callback_extract_opts_t;

int centfs_dirent_callback_extract( centfs_dirent_t *dirent, centfs_dirent_callback_t *callback) {
       	centfs_dirent_callback_extract_opts_t *opts;
	opts=(centfs_dirent_callback_extract_opts_t *)callback->opts;

	/* Skip headers */
	if( dirent->file_idx==0 ) { return(-1); }
	if( !opts->mode_all && (dirent->file.filetype & 0x0f) == CENTFS_FILETYPE_DIR ) { return(-1); };
	centfs_print_dr_file(&dirent->file);
	centfs_dirent_read_file(dirent);


	return(0);
}

int centfs_dir_find( centfs_dirent_t *dirent, centfs_dirent_callback_t *callback, char *match) {
	centfs_dirent_t subdirent, *pardirent;
	uint8_t type;
	int res=0;
	int found=0;
	void *ent;


	if(callback) { callback->count=0; }
	res=centfs_read_dirent_header(dirent);
	if(res < 0 ) { return(res); }


	for (dirent->file_idx=1; (res=centfs_read_dirent_file(dirent))>0; dirent->file_idx++) {
		/* Get file type of this entry */
		type= dirent->file.filetype & 0x0f;
		pardirent= dirent->parent_dirent;

		/* Return  value of  zero indicates a match */
		if( !centfs_fnmatch(dirent->file.filename, match) ) {

			/* Check if a separator was provided in the match string */
			if( centfs_fnsep(match)  ) {
				/* Check if this is a "library file", the term used for a sub directory */
				if( type == CENTFS_FILETYPE_LIB ) {
					/* If so, recurse */
					centfs_dirent_init(
						&subdirent,
						dirent->dev,
						centfs_read_alist_dalent(dirent,(void *)0),
						dirent
					);
					centfs_dir_find(&subdirent,callback,centfs_fnsplit(match));
				} else { continue; }
			/* No separator, get results from this level */
			} else {
				found++;
				if(callback) {
					found += callback->func(dirent,callback);
				}
			}

		} else { continue; }

		//found += callback->func(dirent,callback->opts);

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
	char *match=NULL;
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
	centfs_dirent_callback_t callback;
	centfs_dirent_callback_list_opts_t list_opts = {0};
	centfs_dirent_callback_extract_opts_t extract_opts = {0};

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
		{"dir", SUBCMD_LIST },
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
	} else {
		usage(argv[0]);
	}




	switch(subcmd) {
		case SUBCMD_LIST:
			optstr=list_optstr;
			long_opts=list_long_opts;
			callback.func=&centfs_dirent_callback_list;
			callback.opts=&list_opts;
			break;
		case SUBCMD_EXTRACT:
			optstr=extract_optstr;
			long_opts=extract_long_opts;
			callback.func=&centfs_dirent_callback_extract;
			callback.opts=&extract_opts;
			break;
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
						list_opts.mode_all=1;
						break;
					case 'R':
						break;
					case 'l':
						list_opts.mode_long=1;
						break;
					case 'i':
						list_opts.mode_sector_index=1;
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




	centfs_dirent_init(&dirent, &dev, base_dir_sector, (void *)0);

	if(match) {
		//printf("Searching for '%.10s'\n", match);
		cents_nadd_0x80(match,strnlen(match,10));
	}
	found=centfs_dir_find(&dirent,&callback,match);

	return(0);
	
}
