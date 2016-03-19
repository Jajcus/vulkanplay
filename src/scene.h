#ifndef scene_h
#define scene_h

#include "model.h"
#include <pthread.h>

#define MATERIALS_MAX 16
#define LIGHTS_MAX 16

struct material {
	Vec4 ambient_color;
	Vec4 diffuse_color;
	Vec4 specular_color;
	float shininess, pad1, pad2, pad3;
};

struct light {
	Vec4 position;
	Vec4 diffuse;
	Vec4 specular;
};

struct scene_object {
	struct model * model;
	Mat4 model_matrix;

	/* scene state - set by scene, cleared by renderer */
	struct {
		int matrix_dirty;
		int mesh_dirty;
	} s;

	/* renderer state - mainained by renderer */
	struct {
		uint32_t vertex_index;
		uint32_t instance_index;
		uint32_t index_index;
	} r;
};

struct scene {
	Vec3 eye_pos, eye_dir;

	Vec4 ambient_light;
	const struct light * lights;
	uint32_t lights_len;

	struct scene_object * objects;
	uint32_t objects_len;
	uint32_t objects_size;

	const struct material * materials;
	uint32_t materials_len;

	/* scene state – set by scene, cleared by renderer */
	struct {
		int view_dirty;    /* eye position or direction changed */
		int objects_dirty; /* object list changed - rebuild everything */
		int materials_dirty; /* materials changed */
	} s;

	/* renderer state */
	struct {
		int something; /* FIXME */
	} r;

	/* for synchronization between scene and renderer */
	pthread_mutex_t mutex;
};


struct scene * create_scene(void);

#define scene_lock(scene)   pthread_mutex_lock(&(scene)->mutex)
#define scene_unlock(scene) pthread_mutex_unlock(&(scene)->mutex)

void scene_add_object(struct scene * scene, struct model * model, Mat4 matrix);
void scene_set_eye(struct scene * scene, Vec3 position, Vec3 direction);
void destroy_scene(struct scene * scene);

#endif
