#ifndef plat_h
#define plat_h

#include "vkapi.h"

struct plat_surface {
	VkSurfaceKHR surface;
	void (*event_loop)(struct plat_surface *surf);
	void (*destroy)(struct plat_surface *surf);
};

extern volatile int exit_requested;

#endif
