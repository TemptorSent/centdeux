#include "centfs.h"

const char * const centfs_filetype_list[9][3] = {
	{"0", "dir", "directory file"},
	{"1", "bin", "binary file"},
	{"2", "lnk", "linked sequential file"},
	{"3", "ctg", "contiguous sectored file"},
	{"4", "exf", "executable program file"},
	{"5", "lib", "private library file"},
	{"6", "ind", "hashed indexed file"},
	{"7", "seg", "incomplete file segment"},
	{"8", "que", "spooler que file"}
};

int centfs_device_open(centfs_device_t *dev) {
	return( (dev->state = (dev->device_open)(dev)) );
}

int centfs_device_close(centfs_device_t *dev) {
	return( (dev->state = (dev->device_close)(dev)) );
}


int centfs_read_sector(centfs_device_t *dev, centfs_sector_t *sec) {
	return( (sec->state = (dev->read_sector)(dev,sec)) );
}

