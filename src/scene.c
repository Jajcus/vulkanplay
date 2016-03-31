#include "scene.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "materials.h"

static const struct material MATERIALS[MATERIAL_COUNT] = {
	{ // RED
	.ambient_color = { 0.9f, 0.0f, 0.0f, 1.0f},
	.diffuse_color = { 0.9f, 0.0f, 0.0f, 1.0f},
	.specular_color = { 0.1f, 0.0f, 0.0f, 1.0f},
	.shininess = 10,
	},
	{ // YELLOW
	.ambient_color = { 0.9f, 0.9f, 0.0f, 1.0f},
	.diffuse_color = { 0.5f, 0.5f, 0.0f, 1.0f},
	.specular_color = { 1.0f, 1.0f, 0.5f, 1.0f},
	.shininess = 200,
	},
	{ // GRASS
	.ambient_color = { 0.05f, 0.5f, 0.0f, 1.0f},
	.diffuse_color = { 0.05f, 0.5f, 0.0f, 1.0f},
	.specular_color = { 0.0f, 0.1f, 0.0f, 1.0f},
	.shininess = 10,
	},
	{ // WATER
	.ambient_color = { 0.00f, 0.0f, 0.05f, 1.0f},
	.diffuse_color = { 0.00f, 0.0f, 1.0f, 1.0f},
	.specular_color = { 1.0f, 1.0f, 1.0f, 1.0f},
	.shininess = 100,
	},
	{ // ROCK
	.ambient_color =  { 0.4f, 0.4f, 0.3f, 1.0f},
	.diffuse_color =  { 0.4f, 0.4f, 0.3f, 1.0f},
	.specular_color = { 0.3f, 0.3f, 0.3f, 1.0f},
	.shininess = 5,
	},
	{ // SNOW
	.ambient_color =  { 0.9f, 0.9f, 0.9f, 1.0f},
	.diffuse_color =  { 0.9f, 0.9f, 0.95f, 1.0f},
	.specular_color = { 0.9f, 0.9f, 0.9f, 1.0f},
	.shininess = 10,
	},
	{ // SAND
	.ambient_color = { 0.6f, 0.6f, 0.3f, 1.0f},
	.diffuse_color = { 0.6f, 0.6f, 0.3f, 1.0f},
	.specular_color = { 0.6f, 0.6f, 0.3f, 1.0f},
	.shininess = 1,
	},
};

#define LIGHT_COUNT 1
const static struct light LIGHTS[LIGHT_COUNT] = {
	{
		.position = { 100.0f, 2000.0f, -2000.0f, 1.0f },
		.diffuse = { 1.0f, 1.0f, 1.0f, 1.0f },
		.specular = { 1.0f, 1.0f, 1.0f, 1.0f },
	}
};

const Vec4 AMBIENT_LIGHT = { 0.02f, 0.02f, 0.02, 1.0f };

struct scene * create_scene(void) {

	assert(MATERIAL_COUNT <= MATERIALS_MAX);
	assert(LIGHT_COUNT <= LIGHTS_MAX);

	struct scene * scene = (struct scene *) calloc(1, sizeof(struct scene));
	pthread_mutex_init(&scene->mutex, NULL);

	scene->eye_pos.x =  0.0f;
	scene->eye_pos.y =  0.0f;
	scene->eye_pos.z = -5.0f;

	scene->eye_dir.x =  0.0f;
	scene->eye_dir.y =  0.0f;
	scene->eye_dir.z =  1.0f;

	scene->ambient_light = AMBIENT_LIGHT;
	scene->lights = LIGHTS;
	scene->lights_len = LIGHT_COUNT;

	scene->objects_size = 10;
	scene->objects = (struct scene_object *)calloc(scene->objects_size, sizeof(struct scene_object));
	scene->objects_len = 0;

	scene->materials = MATERIALS;
	scene->materials_len = MATERIAL_COUNT;

	scene->s.view_dirty = 1;
	scene->s.objects_dirty = 1;
	scene->s.materials_dirty = 1;

	return scene;
}

void scene_add_object(struct scene * scene, struct model * model, Mat4 matrix) {

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

	obj->model_matrix = matrix;
	obj->model = model;

	obj->s.matrix_dirty = 1;
	obj->s.mesh_dirty = 1;

	scene->s.objects_dirty = 1;
	scene_unlock(scene);
}

void scene_set_eye(struct scene * scene, Vec3 position, Vec3 direction) {

	scene_lock(scene);
	scene->eye_pos = position;
	scene->eye_dir = direction;
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

