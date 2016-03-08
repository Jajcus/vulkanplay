#ifndef plat_h
#define plat_h

#include <stdint.h>
#include <stdbool.h>

extern struct options {
	bool fullscreen;
	bool pres_mode;
	bool stats;
	float fps_cap;

	uint32_t win_width;
	uint32_t win_height;
} options;

void request_exit(void);
bool exit_requested(void);

#endif
