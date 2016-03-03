#ifndef renderer_h
#define renderer_h

struct renderer * start_renderer(struct plat_surface * surface);
void stop_renderer(struct renderer * renderer);

#endif
