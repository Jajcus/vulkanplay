#ifndef surface_h
#define surface_h

#include "vkapi.h"

struct plat_surface {
	void (*event_loop)(struct plat_surface *surf);
	void (*destroy)(struct plat_surface *surf);

	// Vulkan surface object
	VkSurfaceKHR vk_surface;

	// Vulkan surface capabilities
	uint32_t s_formats_count;
	VkSurfaceFormatKHR * s_formats;
	VkFormat s_format;
	VkColorSpaceKHR s_colorspace;
	uint32_t s_modes_count;
	VkPresentModeKHR * s_modes;

	// Suggested size, if not provided by vk_surface
	uint32_t width;
	uint32_t height;
};

void finalize_surface(struct plat_surface * surface);

#endif
