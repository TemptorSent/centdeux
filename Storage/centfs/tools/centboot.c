#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NATIVE_BLOCK_SIZE 400
#define SECTORS_PER_TRACK 16

static void usage(const char *progname) {
    printf("Usage: %s <input file> [512]\n", progname);
    exit(255);
}

FILE *f;
unsigned int block_size = NATIVE_BLOCK_SIZE;
static unsigned char buffer[512];

static void readSector(unsigned int pos)
{
    pos *= block_size;

    if (fseek(f, pos, SEEK_SET)) {
	fprintf(stderr, "Failed to seek to 0x%08x: %s\n", pos, strerror(errno));
	fclose(f);
	exit(255);
    }
    
    int b = fread(buffer, 1, block_size, f);

    if (b == 0) {
	fprintf(stderr, "Image file truncated; offset %u does not exist\n", pos);
	fclose(f);
	exit(255);
    }

    if (b == -1) {
	fprintf(stderr, "Failed to read at offset 0x%08x: %s\n", pos, strerror(errno));
	fclose(f);
	exit(255);
    }

    if (b < NATIVE_BLOCK_SIZE) {
        printf("WARNING! Last block truncated (%u bytes)!\n", b);
    }
}
 
static unsigned short read_be16(unsigned int offset)
{
    return buffer[offset] << 8 | buffer[offset + 1];
}

/*
 * Original algorithm from WIPL bootloader:
 *
 * 0266:    95 88 06     ld AX, +0x6(EX)	 ; Disk code is derived from this value
 * 0269:    3b           not! AX
 * 026a:    c0 80        ld BL, #0x80	 ; This whole thing rotates AX right WITHOUT carry
 * 026c:    07           rl
 * 026d:    36 00        rrc AX, 1
 * 026f:    11 02        bnc L_0273
 * 0271:    43 30        or AH, BL
 * 
 * L_0273:
 * 0273:    d0 3c b1     ld BX, #0x3cb1	 ; Some more obfuscation
 * 0276:    44 32        xor BH, BL
 * 0278:    54 02        xor BX, AX	 ; BX is the final expected value here
 */
static unsigned short make_key(unsigned short seed)
{
    unsigned short AX = ~seed;
    unsigned short rotated = (AX >> 1) | (AX << 15);
    unsigned short BH = 0x3c ^ 0xb1;
    unsigned short BX = (BH << 8) | 0xb1;
    
    return BX ^ rotated;
}

/*
 * Original algorithm from WIPL bootloader:
 *
 * 0293:    95 88 04     ld AX, +0x4(EX)	 ; sector_base + 4
 * 0296:    44 10        xor AH, AL
 * 0298:    d0 3c b1     ld BX, #0x3cb1	 ; The same magic constant as used for making the code
 * 029b:    54 02        xor BX, AX
 * 029d:    91 01 05     ld AX, (0x0105)	 ; entered_disk_code, which we now know is correct
 * 02a0:    50 20        add AX, BX	 ; More mathemagic
 * 02a2:    35 03        sll AX, 4
 */
static unsigned short calc_boot_dir_start(unsigned short encrypted, unsigned short code)
{
    unsigned short AX = encrypted ^ (encrypted << 8);
    unsigned short BX = 0x3cb1 ^ AX;
 
    return (code + BX) * SECTORS_PER_TRACK;
}
 
static int is_text(unsigned int offset, unsigned int length)
{
    unsigned int i;

    for (i = 0; i < length; i++)
    {
	if (!((buffer[i] & 0x80) && isprint(buffer[i] & 0x7F))) {
	    return 0;
	}
    }

    return 1;
}

static void strip0x80(char *buf, const unsigned char *str, unsigned int n)
{
    unsigned int i;

    for (i = 0; i < n; i++) {
	*buf++ = *str++ & 0x7f;
    }
    *buf = 0;
}

#define FILENAME_LENGTH 10

static unsigned int num_files = 0;

static int findBootFiles(unsigned int offset)
{
    while (offset < NATIVE_BLOCK_SIZE)
    {
        if (read_be16(offset) == 0x848d) {
	    return 0;
	}

	if (is_text(offset, FILENAME_LENGTH)) {
	    char name[FILENAME_LENGTH + 1];
	    
	    strip0x80(name, buffer + offset, FILENAME_LENGTH);
	    /*
	     * The second number here is sector number, from which supposedly
	     * the file starts. The loading procedure isn't trivial, something like
	     * block list may be involved.
	     * I don't know the rest of details yet.
	     */
	    printf("%s 0x%02x %u 0x%02x\n", name, buffer[offset + FILENAME_LENGTH],
                                                  read_be16(offset + FILENAME_LENGTH + 1),
 					          buffer[offset + FILENAME_LENGTH + 3]);
            num_files++;
	}

	offset += 16;
    }

    return 1;
}

/*
 * This tool lists known boot data (file names and disk code)
 * Reverse engineed from the IPL bootstrap by Pavel Fedin <pavel_fedin@mail.ru>
 */
int main(int argc, const char **argv) {
    unsigned int sector;
    unsigned int name_offset;
    unsigned short key;

    if (argc < 2) {
	usage(argv[0]);
    }

    if (argc > 2 && !strcmp(argv[2], "512")) {
	printf("Using 512 bytes per block (CCDP format)\n");
	block_size = 512;
    }

    f = fopen(argv[1], "rb");
    if (!f) {
	 perror("Failed to open input file");
	 return 255;
    }

    readSector(14);
    key = make_key(read_be16(6));

    /* Dump what we know from the header */
    printf("Disk format flag: 0x%02x - %s\n", buffer[8], buffer[8] == 0xFF ? "Correct" : "WRONG!");
    printf("Disk code is: %u\n", key);

    /* Here we fully replicate what the IPL does */    
    sector = calc_boot_dir_start(read_be16(4), key);
    name_offset = 16;

    /*
     * Starting from the "sector" there is a list of bootable files,
     * 16 bytes per entry. Let's call this a "boot directory".
     * The first entry is skipped, from the image we have we suppose that's a
     * volume name.
     */
    printf("Boot directory start sector: %u\n", sector);
    
    readSector(sector);

    /* When loading a boot file, the WIPL treats all sector numbers
       in the boot entry as relative to this. Let's call it a boot partition. */
    printf("Boot partition start sector: %u\n", read_be16(14));

    /*
     * The IPL follows this exact algorithm searching for a boot file.
     * It just keeps reading through sectors until it finds the file or
     * hits the terminator word.
     * In the image we have this terminator word precedes what we think is a
     * data file entry.
     */
    while (findBootFiles(name_offset)) {
	readSector(++sector);
	name_offset = 0;
    }

    if (!num_files) {
	printf("Sorry, no bootable files on this disk\n");
    }

    fclose(f);
    return 0;
}