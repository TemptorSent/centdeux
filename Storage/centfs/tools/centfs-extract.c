#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "common-cents.h"
#include "centfs-disk-format.h"
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
	dev->internal=(void *)img;
	dev->device_open= &centfs_image_device_open;
	dev->device_close= &centfs_image_device_close;
	dev->read_sector= &centfs_image_read_sector;
	img->filename= filename;
	img->sector_size= sector_size;
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
			: drsec.number
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
			centfs_sector_byte_t next_dalent = {.sector=0, .byte=0};
			subdirent.parent_dirent=dirent;
			subdirent.dev=dirent->dev;
			subdirent.base_sector=centfs_read_alist_dalent(dirent,&next_dalent);
			centfs_dir_list(&subdirent);
		}


		centfs_print_alist_dal(dirent);
		printf("\n");


	}
	if( res < 1 ) { return(res); }
	return(dirent->file_idx);
}
/*
	while( !( (dirent->dev->read_sector)(dirent->dev,&drsec) ) ) {
		if(i==0) {
			centfs_read_dirent_header(dirent);
			if(!dirent->parent_dirent) { centfs_print_dr_header(&dirent->header); }
			i++;
			continue;
		}

		do {
			int res;
			dirent->file_idx=i;
			res=centfs_read_dirent_file(dirent);
			if( res < 1 ) { return(-1); }
			else if ( res == 0 ) { goto centfs_dir_list_done; }
			centfs_read_alist_attr(dirent);

			if(dirent->parent_dirent) { printf("  "); }
			centfs_print_dr_file(&dirent->file);
			centfs_print_alist_attr(&dirent->attr);
			printf("\n");

			if( (dirent->file.filetype & 0x0f ) == 0x5 ) {
				centfs_sector_byte_t next_dalent = {.sector=0, .byte=0};
				subdirent.parent_dirent=dirent;
				subdirent.dev=dirent->dev;
				subdirent.base_sector=centfs_read_alist_dalent(dirent,&next_dalent);
				centfs_dir_list(&subdirent);
			}


			//centfs_print_alist_dal(dirent);

			i++;
		} while ( i % (CENTFS_BYTES_PER_SECTOR/CENTFS_DR_LENGTH) );

		drsec.number++;	
	}
*/
	/* If we reached here, we had a read error on a sector */
	/*
	return(-1);

centfs_dir_list_done:
	return(i);

}
*/


int main(int argc, char **argv) {
	centfs_image_file_t img;
	centfs_dr_header_t *dr0;
	centfs_dr_file_t *dr;
	centfs_sector_t drsec,mapsec;
	centfs_device_t dev;
	centfs_sector_number_t alist_sector_start;
	centfs_dirent_t dirent;
	
	
	char dirname[sizeof(dr->filename)];

	uint32_t tsec, nsec, n;

	if(argc < 2 ) {
		printf("Requires filename as argument.\n");
		return(-EINVAL);
	}
	centfs_image_device_init(&dev,&img,argv[argc-1],CENTFS_BYTES_PER_SECTOR);


	if(argc > 2) { nsec=strtol(argv[1],NULL,0); }
	else { nsec=0x10; }

	(dev.device_open)(&dev);

	dirent.dev=&dev;
	dirent.base_sector=nsec;
	dirent.parent_dirent=0;
	centfs_dir_list(&dirent);

	return(0);
	
}
