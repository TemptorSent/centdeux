#pragma once
#include <stdint.h>
#include "centfs-disk-format.h"

#define CCDP_DUMP_BYTES_PER_SECTOR 0x200 // 512 Bytes per Sector;
#define CCDP_DUMP_SECTORS_PER_TRACK CENTFS_SECTORS_PER_TRACK // 16 Sectors per Track
#define CCDP_DUMP_BYTES_PER_TRACK ( CCDP_DUMP_BYTES_PER_SECTOR * CCDP_DUMP_SECTORS_PER_TRACK ) // 8192 Bytes per Track

#define CCDP_DUMP_SECTOR_TO_TRACK(sector) CENTFS_SECTOR_TO_TRACK(sector)
#define CCDP_DUMP_TRACK_FIRST_SECTOR(track) CENTFS_TRACK_FIRST_SECTOR(track)
#define CCDP_DUMP_TRACK_LAST_SECTOR(track) CENTFS_TRACK_LAST_SECTOR(track)

#define CCDP_DUMP_BYTE_ADDRESS_TO_SECTOR(byte) ( (byte) / CCDP_DUMP_BYTES_PER_SECTOR )
#define CCDP_DUMP_BYTE_ADDRESS_TO_SECTOR_BYTE_OFFSET(byte) ( (byte) % CCDP_DUMP_BYTES_PER_SECTOR )

#define CCDP_DUMP_SECTOR_FIRST_BYTE_ADDRESS(sector) ( (sector) * CCDP_DUMP_BYTES_PER_SECTOR )
#define CCDP_DUMP_SECTOR_LAST_BYTE_ADDRESS(sector) ( ( (sector + 1) * CCDP_DUMP_BYTES_PER_SECTOR ) - 1 )

#define CCDP_DUMP_BYTE_ADDRESS_TO_TRACK(byte) ( byte / CCDP_DUMP_BYTES_PER_TRACK )
#define CCDP_DUMP_BYTE_ADDRESS_TO_TRACK_BYTE_OFFSET(byte) ( byte % CCDP_DUMP_BYTES_PER_TRACK )

#define CCDP_DUMP_TRACK_FIRST_BYTE_ADDRESS(track) ( track * CCDP_DUMP_BYTES_PER_TRACK )
#define CCDP_DUMP_TRACK_LAST_BYTE_ADDRESS(track) ( ( (track + 1) * CCDP_DUMP_BYTES_PER_TRACK ) - 1 )

#define CCDP_DUMP_TO_CENTFS_BYTE_ADDRESS(byte) ( \
	(CCDP_DUMP_BYTE_ADDRES_TO_SECTOR(byte) * CENTFS_BYTES_PER_SECTOR) \
	 + CCDP_DUMP_BYTE_ADDRESS_TO_SECTOR_BYTE_OFFSET(byte) \
)

#define CENTFS_TO_CCDP_DUMP_BYTE_ADDRESS(byte) ( \
	(CENTFS_BYTE_ADDRES_TO_SECTOR(byte) * CCDP_DUMP_BYTES_PER_SECTOR) \
	 + CENTFS_BYTE_ADDRESS_TO_SECTOR_BYTE_OFFSET(byte) \
)

typedef uint32_t ccdp_dump_byte_address_t;
typedef uint32_t ccdp_dump_sector_number_t;
typedef uint16_t ccdp_dump_sector_byte_offset_t;
typedef uint16_t ccdp_dump_track_number_t;
typedef uint16_t ccdp_dump_track_byte_offset_t;

