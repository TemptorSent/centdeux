#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#else
#define setmode(x, y)
#endif


#define CCDP_SECTOR_SIZE 512
#define NATIVE_SECTOR_SIZE 400

static void usage(const char *progname) {
    printf("Usage: %s [input file] [output file] [block count]\n", progname);
    printf("       If [output file] is not given, stdout is used\n");
    printf("       If [input file] is not given, stdin and stdout are used\n");
    exit(255);
}

static unsigned char buffer[CCDP_SECTOR_SIZE];

int main(int argc, const char **argv) {
    char *end = NULL;
    const char *in_name = NULL;
    const char *out_name = NULL;
    FILE *in, *out;
    unsigned int count = -1;
    unsigned int bad = 0;
    unsigned int i;

    if (argc > 1) {
	if (!(strcmp(argv[1], "--help") && strcmp(argv[1], "-h"))) {
	    usage(argv[0]);
	}
	in_name = argv[1];
    }

    if (argc > 2) {
	out_name = argv[2];
    }

    if (argc > 3) {
	count = strtoul(argv[3], NULL, 0);
	if (count == 0) {
	    printf("Invalid block count provided!\n");
	    return 255;
	}
    }

    if (in_name) {
        in = fopen(argv[1], "rb");
        if (!in) {
	    perror("Failed to open input file");
	    return 255;
	}
    } else {
	in = stdin;
        setmode(fileno(stdin), O_BINARY);
    }

    if (out_name) {
        out = fopen(argv[2], "wb");
        if (!out) {
	    perror("Failed to open output file");
	    fclose(in);
	    return 255;
	}
    } else {
	out = stdout;
        setmode(fileno(stdout), O_BINARY);
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
	    fprintf(stderr, "WARNING! Last sector truncated (%u bytes)!\n", b);
	} else if (b > NATIVE_SECTOR_SIZE && buffer[NATIVE_SECTOR_SIZE] != 0xFF) {
	    fprintf(stderr, "WARNING! Bad sector 0x%06x!\n", i);
	    bad++;
	}

	b = fwrite(buffer, NATIVE_SECTOR_SIZE, 1, out);

	if (b == -1) {
	    perror("Failed to write output!\n");
	    break;
	}
    }

    if (out != stdout) {
	printf("%u blocks done; %u bad\n", i, bad);
        fclose(out);
    }
    if (in != stdin) {
        fclose(in);
    }

    return 0;
}
