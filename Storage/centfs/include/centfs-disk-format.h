#pragma once
#include <stdint.h>

/*
* Copy this block for each new device type for consistency.
*
* s/MYDEV/<YOURDEV>/g
*
* Update entries at end with device specifics
*
*


*
* These are common entries referring to the CENTFS standard format.
* Change at your own peril!
*
#define MYDEV_BYTES_PER_SECTOR CENTFS_BYTES_PER_SECTOR // 400 Bytes per Sector;
#define MYDEV_SECTORS_PER_TRACK CENTFS_SECTORS_PER_TRACK // 16 Sectors per Track
#define MYDEV_BYTES_PER_TRACK CENTFS_BYTES_PER_TRACK // 6400 Bytes per Track
#define MYDEV_SECTOR_TO_TRACK(sector) CENTFS_SECTOR_TO_TRACK(sector)
#define MYDEV_TRACK_FIRST_SECTOR(track) CENTFS_TRACK_FIRST_SECTOR(track)
#define MYDEV_TRACK_LAST_SECTOR(track) CENTFS_TRACK_LAST_SECTOR(track)
#define MYDEV_BYTE_ADDRESS_TO_SECTOR(byte) CENTFS_BYTE_ADDRESS_TO_SECTOR(byte)
#define MYDEV_BYTE_ADDRESS_TO_TRACK_BYTE_OFFSET(byte) CENTFS_BYTE_ADDRESS_TO_TRACK_BYTE_OFFSET(byte)
#define MYDEV_SECTOR_FIRST_BYTE_ADDRESS(sector) CENTFS_SECTOR_FIRST_BYTE_ADDRESS(sector)
#define MYDEV_SECTOR_LAST_BYTE_ADDRESS(sector) CENTFS_SECTOR_LAST_BYTE_ADDRESS(sector)
#define MYDEV_BYTE_ADDRESS_TO_TRACK(byte) CENTFS_BYTE_ADDRESS_TO_TRACK(byte)
#define MYDEV_BYTE_ADDRESS_TO_TRACK_BYTE_OFFSET(byte) CENTFS_BYTE_ADDRESS_TO_TRACK_BYTE_OFFSET(byte)
#define MYDEV_TRACK_FIRST_BYTE_ADDRESS(track) CENTFS_TRACK_FIRST_BYTE_ADDRESS(track)
#define MYDEV_TRACK_LAST_BYTE_ADDRESS(track) CENTFS_TRACK_LAST_BYTE_ADDRESS(track)

*
* These need the relevant C,H,S parameters filled in:

#define MYDEV_CYLS <num cyls> // <num> Cylinders
#define MYDEV_HEADS_PER_CYL <num heads> // <num> Heads
#define MYDEV_TRACKS ( MYDEV_CYLS * MYDEV_HEADS_PER_CYL ) // <num> Tracks
#define MYDEV_SECTORS ( MYDEV_TRACKS * MYDEV_SECTORS_PER_TRACK ) // <num> Sectors
#define MYDEV_BYTES ( MYDEV_SECTORS * MYDEV_BYTES_PER_SECTOR ) // <num> Bytes

#define MYDEV_CHS_C_BITPOS [C field beginning bit position]
#define MYDEV_CHS_H_BITPOS [H field beginning bit position]
#define MYDEV_CHS_S_BITPOS [S field beginning bit position]

#define MYDEV_CHS_C_BITMASK [C field bit mask]
#define MYDEV_CHS_H_BITMASK [H field bit mask]
#define MYDEV_CHS_S_BITMASK [S field bit mask]

#define MYDEV_SECTOR_TO_CHS_C(sector) ( (sector>>MYDEV_CHS_C_BITPOS) & MYDEV_CHS_C_BITMASK )
#define MYDEV_SECTOR_TO_CHS_H(sector) ( (sector>>MYDEV_CHS_H_BITPOS) & MYDEV_CHS_H_BITMASK )
#define MYDEV_SECTOR_TO_CHS_S(sector) ( (sector>>MYDEV_CHS_S_BITPOS) & MYDEV_CHS_S_BITMASK )
#define MYDEV_CHS_TO_SECTOR(c,h,s) ( (c<<MYDEV_CHS_C_BITPOS) | (h<<MYDEV_CHS_H_BITPOS) | (s<<MYDEV_CHS_S_BITPOS) )

*/

#define CENTFS_BYTES_PER_SECTOR 0x190 // 400 Bytes per Sector;
#define CENTFS_SECTORS_PER_TRACK 0x10 // 16 Sectors per Track
#define CENTFS_BYTES_PER_TRACK ( CENTFS_BYTES_PER_SECTOR * CENTFS_SECTORS_PER_TRACK ) // 6400 Bytes per Track



#define CENTFS_SECTOR_TO_TRACK(sector) ( sector / CENTFS_SECTOR_PER_TRACK )
#define CENTFS_TRACK_FIRST_SECTOR(track) ( track * CENTFS_SECTORS_PER_TRACK )
#define CENTFS_TRACK_LAST_SECTOR(track) ( ( (track + 1) * CENTFS_SECTORS_PER_TRACK ) - 1 )


#define CENTFS_BYTE_ADDRESS_TO_SECTOR(byte) ( byte / CENTFS_BYTES_PER_SECTOR )
#define CENTFS_BYTE_ADDRESS_TO_SECTOR_BYTE_OFFSET(byte) ( byte % CENTFS_BYTES_PER_SECTOR )

#define CENTFS_SECTOR_FIRST_BYTE_ADDRESS(sector) ( sector * CENTFS_BYTES_PER_SECTOR )
#define CENTFS_SECTOR_LAST_BYTE_ADDRESS(sector) ( ( (sector + 1) * CENTFS_BYTES_PER_SECTOR ) - 1 )

#define CENTFS_BYTE_ADDRESS_TO_TRACK(byte) ( byte / CENTFS_BYTES_PER_TRACK )
#define CENTFS_BYTE_ADDRESS_TO_TRACK_BYTE_OFFSET(byte) ( byte % CENTFS_BYTES_PER_TRACK )

#define CENTFS_TRACK_FIRST_BYTE_ADDRESS(track) ( track * CENTFS_BYTES_PER_TRACK )
#define CENTFS_TRACK_LAST_BYTE_ADDRESS(track) ( ( (track + 1) * CENTFS_BYTES_PER_TRACK ) - 1 )


typedef uint32_t centfs_byte_address_t;
typedef uint32_t centfs_sector_number_t;
typedef uint16_t centfs_sector_byte_offset_t;
typedef uint16_t centfs_track_number_t;
typedef uint16_t centfs_track_byte_offset_t;

