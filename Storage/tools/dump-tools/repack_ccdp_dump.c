#include <stdio.h>
#include <stdlib.h>

#define CCDP_SECTOR_SIZE 512
#define NATIVE_SECTOR_SIZE 400

static void usage(const char *progname) {
    printf("Usage: %s <input file> <output file> [block count]\n", progname);
    exit(255);
}

static unsigned char buffer[CCDP_SECTOR_SIZE];

int main(int argc, const char **argv) {
    char *end = NULL;
    FILE *in, *out;
    unsigned int count = -1;
    unsigned int bad = 0;
    unsigned int i;

    if (argc < 3) {
	usage(argv[0]);
    }

    if (argc > 3) {
	count = strtoul(argv[3], NULL, 0);
	if (count == 0) {
	    printf("Invalid block count provided!\n");
	    return 255;
	}
    }

    in = fopen(argv[1], "rb");
    if (!in) {
	 perror("Failed to open input file");
	 return 255;
    }

    out = fopen(argv[2], "wb");
    if (!out) {
	perror("Failed to open output file");
	fclose(in);
	return 255;
    }

    for (i = 0; i < count; i++) {
	int b = fread(buffer, 1, CCDP_SECTOR_SIZE, in);

	if (b == 0) {
	    break; /* EOF */
	}

	if (b == -1) {
	    perror("Failed to read input!\n");
	    break;
	}

	if (b < NATIVE_SECTOR_SIZE) {
	    printf("WARNING! Last sector truncated (%u bytes)!\n", b);
	} else if (b > NATIVE_SECTOR_SIZE && buffer[NATIVE_SECTOR_SIZE] != 0xFF) {
	    printf("WARNING! Bad sector 0x%06x!\n", i);
	    bad++;
	}

	b = fwrite(buffer, NATIVE_SECTOR_SIZE, 1, out);

	if (b == -1) {
	    perror("Failed to write output!\n");
	    break;
	}
    }

    printf("%u blocks done; %u bad\n", i, bad);

    fclose(out);
    fclose(in);

    return 0;
}
