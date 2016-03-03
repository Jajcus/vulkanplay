#include "surface.h"
#include <malloc.h>

void finalize_surface(struct plat_surface * surface) {

	if (surface->vk_surface) {
		vkDestroySurfaceKHR(vkapi.instance, surface->vk_surface, NULL);
		surface->vk_surface = VK_NULL_HANDLE;
	}
	if (surface->s_formats) {
		free(surface->s_formats);
		surface->s_formats_count = 0;
	}
	if (surface->s_modes) {
		free(surface->s_modes);
		surface->s_modes_count = 0;
	}
}
