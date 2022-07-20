#include <sys/types.h>

#include "centfs-disk-format.h"

#define CENTFS_FILENAME_BYTES 10
#define CENTFS_SECTORNUM_BYTES 3
#define CENTFS_DATESTAMP_BYTES 2

enum CENTFS_SECTOR_IOSTATE {
	CENTFS_SECTOR_IOSTATE_UNREAD=0x0,
	CENTFS_SECTOR_IOSTATE_READ_VALID=0x1,
	CENTFS_SECTOR_IOSTATE_SEEK_ERROR,
	CENTFS_SECTOR_IOSTATE_READ_ERROR
};

typedef struct centfs_sector_t {
	uint8_t data[CENTFS_BYTES_PER_SECTOR];	
	centfs_sector_number_t number;
	enum CENTFS_SECTOR_IOSTATE state;
} centfs_sector_t;


enum CENTFS_DEVICE_IOSTATE {
	CENTFS_DEV_IOSTATE_NODEV,
	CENTFS_DEV_IOSTATE_OPEN,
	CENTFS_DEV_IOSTATE_CLOSED,
	CENTFS_DEV_IOSTATE_IOERROR
};

typedef struct centfs_device_t {
	int (*device_open)(struct centfs_device_t *dev);
	int (*device_close)(struct centfs_device_t *dev);
	int (*read_sector)(struct centfs_device_t *dev, centfs_sector_t *sec);
	void *internal;
	enum CENTFS_DEVICE_IOSTATE state;
} centfs_device_t;


typedef uint8_t centfs_sector_addr_t[CENTFS_SECTORNUM_BYTES];
typedef char centfs_filename_t[CENTFS_FILENAME_BYTES];
typedef uint8_t centfs_datestamp_t[CENTFS_DATESTAMP_BYTES];

typedef uint16_t centfs_sector_byte_offset_t;

typedef struct centfs_sector_byte_t {
	centfs_sector_number_t sector;
	centfs_sector_byte_offset_t byte;
} centfs_sector_byte_t;


#define CENTFS_DR_LENGTH 0x10

#define CENTFS_DRS_PER_SECTOR ( CENTFS_BYTES_PER_SECTOR / CENTFS_DR_LENGTH )


/* From RPL2 dump */

/*
*
*        directory record zero
*
drvol    equ    0            name of disk volume
drrgdt   equ    10           date of last reorg
drrger   equ    12           number of errors in last reorg
draloc   equ    13           disk address of first sector of alloc list file
*
*/

typedef union centfs_dr_header_t {
	struct {
		centfs_filename_t drvol;
		centfs_datestamp_t drrgdt;
		uint8_t drrger;
		centfs_sector_addr_t draloc;
	};
	struct {
		centfs_filename_t volume_name;
		centfs_datestamp_t last_reorg_date;
		uint8_t last_reorg_errors;
		centfs_sector_addr_t alist_sector_start;
	};
} centfs_dr_header_t;


/*
*
*        directory displacements
*
drfnam   equ    0            file name
drabp    equ    10           buffer pointer/3 of allocation list for file
draad    equ    11           relative disk address of allocation list for file
dratt    equ    13           file attributes: 4-bits unused/4-bits file type
drdat    equ    14           binary file date
drlen    equ    16           length of directory entry
*
*/


typedef union centfs_dr_file_t {
	struct {
		centfs_filename_t drfnam;
		uint8_t drabp;
		uint8_t draad[2];
		uint8_t dratt;
		centfs_datestamp_t drdat;
	};
	struct {
		centfs_filename_t filename;
		uint8_t alist_entry_number;
		uint8_t alist_sector_offset[2];
		uint8_t filetype;
		centfs_datestamp_t date;
	};
} centfs_dr_file_t;



typedef union centfs_dr_t {
	centfs_dr_header_t header;
	centfs_dr_file_t file;
} centfs_dr_t;

/*
*
*        attribute list record displacements
*
atsiz    equ    0            file size
atbbp    equ    2            file beginning buffer pointer
atfsi    equ    4            file size incriment
atcls    equ    5            file class
atdal    equ    6            file disk address list
atflen   equ    atdal        fcb fixed length
*
*/

#define CENTFS_ALIST_ATTR_LENGTH 0x6

typedef union centfs_alist_attr_t {
	struct {
		uint8_t atsiz[2];
		uint8_t atbbp[2];
		uint8_t atfsi;
		uint8_t atcls;
	};
	struct {
		uint8_t filesize[2];
		uint8_t file_begin_pointer[2];
		uint8_t filesize_increment;
		uint8_t fileclass;
	};
} centfs_alist_attr_t;

#define CENTFS_ALIST_DALENT_LENGTH 0x3

typedef centfs_sector_addr_t centfs_alist_dalent_t;


typedef struct centfs_dirent_t {
	centfs_device_t *dev;
	centfs_sector_number_t	base_sector;
	centfs_dr_header_t	header;
	uint16_t		file_idx; /* Hopefully we can't have more than 64k files in a dir */	
	centfs_sector_byte_t	file_ptr;
	centfs_dr_file_t	file;
	centfs_sector_byte_t	attr_ptr;
	centfs_alist_attr_t	attr;
	centfs_sector_byte_t	dal_ptr;
	struct centfs_dirent_t	*parent_dirent;

} centfs_dirent_t;

typedef struct centfs_FILE {
	centfs_dirent_t *dirent;
	centfs_sector_t sector;
	off_t pos_offset;
	centfs_sector_byte_t pos_sector_byte;
	centfs_sector_number_t blksize;
	centfs_sector_number_t blknum;
	centfs_sector_byte_t curr_dalent;
	centfs_sector_byte_t next_dalent;




} centfs_FILE;

#define CENTFS_FILETYPE_DIR	0x0
#define CENTFS_FILETYPE_BIN	0x1
#define CENTFS_FILETYPE_LNK	0x2
#define CENTFS_FILETYPE_CTG	0x3
#define CENTFS_FILETYPE_EXF	0x4
#define CENTFS_FILETYPE_LIB	0x5
#define CENTFS_FILETYPE_IND	0x6
#define CENTFS_FILETYPE_SEG	0x7
#define CENTFS_FILETYPE_QUE	0x8
#define CENTFS_FILETYPE_MAX	0x8


extern const char * const centfs_filetype_list[CENTFS_FILETYPE_MAX + 1][3];

