#include "scene.h"
#include <malloc.h>
#include <string.h>

#define MATERIAL_COUNT 1

static const struct material materials[MATERIAL_COUNT] = {
	{
	.color = { 1.0f, 0.0f, 0.0f, 1.0f},
	},
};

struct scene * create_scene(void) {

	struct scene * scene = (struct scene *) calloc(1, sizeof(struct scene));
	pthread_mutex_init(&scene->mutex, NULL);

	scene->eye[0] = -0.5f;
	scene->eye[1] =  1.0f;
	scene->eye[2] =  6.0f;

	scene->light_pos[0] =  2.0f;
	scene->light_pos[1] =  2.0f;
	scene->light_pos[2] = 10.0f;
	scene->light_pos[3] =  1.0f;

	scene->objects_size = 10;
	scene->objects = (struct scene_object *)calloc(scene->objects_size, sizeof(struct scene_object));
	scene->objects_len = 0;

	scene->materials = materials;
	scene->materials_len = MATERIAL_COUNT;

	scene->s.view_dirty = 1;
	scene->s.objects_dirty = 1;
	scene->s.materials_dirty = 1;

	return scene;
}

void scene_add_object(struct scene * scene, struct model * model, mat4x4 matrix) {

	scene_lock(scene);
	if (scene->objects_len == scene->objects_size) {
		uint32_t old_size = scene->objects_size;
		uint32_t added = scene->objects_size / 2;
		scene->objects_size = old_size + added;
		scene->objects = realloc(scene->objects, scene->objects_size);
		memset(scene->objects + old_size, 0, added * sizeof(struct scene_object));
	}
	uint32_t i = scene->objects_len++;
	struct scene_object * obj = &scene->objects[i];

	memcpy(obj->model_matrix, matrix, sizeof(mat4x4));
	obj->model = model;

	obj->s.matrix_dirty = 1;
	obj->s.mesh_dirty = 1;

	scene->s.objects_dirty = 1;
	scene_unlock(scene);
}

void scene_set_eye(struct scene * scene, vec3 eye, vec3 look_at) {

	scene_lock(scene);
	memcpy(scene->eye, eye, sizeof(vec3));
	memcpy(scene->look_at, look_at, sizeof(vec3));
	scene_unlock(scene);
}

void destroy_scene(struct scene * scene) {

	uint32_t i;

	if (!scene) return;
	if (scene->objects) {
		for(i = 0; i < scene->objects_len; i++) {
			destroy_model(scene->objects[i].model);
		}
		free(scene->objects);
	}
	free(scene);
}

