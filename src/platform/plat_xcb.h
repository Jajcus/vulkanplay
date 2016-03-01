#ifndef plat_xcb
#include "plat.h"

struct plat_surface * plat_xcb_get_surface(void);
void plat_xcb_destroy_surface(struct plat_surface *surf);
void plat_xcb_event_loop(struct plat_surface *surf);

#endif
