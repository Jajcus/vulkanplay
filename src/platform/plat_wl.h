#ifndef plat_wl
#include "main.h"

struct plat_surface * plat_wl_get_surface(void);
void plat_wl_destroy_surface(struct plat_surface *surf);
void plat_wl_event_loop(struct plat_surface *surf);

#endif
