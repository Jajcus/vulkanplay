#ifndef renderer_h
#define renderer_h

struct scene;
struct renderer * start_renderer(struct plat_surface * surface, struct scene * scene);
void stop_renderer(struct renderer * renderer);

#endif
