#ifndef plat_h
#define plat_h

#include <stdint.h>

extern struct options {
	int fullscreen;
	int pres_mode;
	int stats;
	float fps_cap;

	uint32_t win_width;
	uint32_t win_height;
} options;

void request_exit(void);
int exit_requested(void);

#endif
