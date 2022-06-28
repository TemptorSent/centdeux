#include "centfs.h"


int centfs_device_open(centfs_device_t *dev) {
	return( (dev->state = (dev->device_open)(dev)) );
}

int centfs_device_close(centfs_device_t *dev) {
	return( (dev->state = (dev->device_close)(dev)) );
}


int centfs_read_sector(centfs_device_t *dev, centfs_sector_t *sec) {
	return( (sec->state = (dev->read_sector)(dev,sec)) );
}

