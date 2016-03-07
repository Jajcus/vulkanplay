#ifndef world_h
#define world_h

struct world;
struct scene;

struct world * create_world(void);
struct scene * world_get_scene(struct world * world);
void destroy_world(struct world *);

#endif
