#include <stdint.h>
typedef struct centar_dirent_t {
	char volname[10];
	uint8_t b0a_0f[6];
	uint8_t filemark[2];
	char filename[10];
	uint8_t b1c_1e[3];
	uint8_t filetype;
	uint8_t b20_2d[13];
	uint8_t next_sector[2];
	uint8_t b2f;
	char diskname[10];
	char seqnum[2];
	uint8_t b3d_3f[4];
} centar_dirent_t;

