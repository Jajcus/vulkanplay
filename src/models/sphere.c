
#include "models/sphere.h"

#include <malloc.h>
#include <string.h>

struct sphere_model {
       struct model model;

       int detail;
       float radius;
};

struct model * create_sphere(uint32_t material, int detail, float radius) {

	if (detail < 1) detail = 1;

	struct sphere_model * sphere = (struct sphere_model *)calloc(1, sizeof(struct sphere_model));

	sphere->model.type = SPHERE_MODEL;

	int par = detail;
	int mer = detail + 2;

	int vert_count = 2 + mer * par;

	sphere->model.vertices = (struct vertex_data *)calloc(vert_count, sizeof(struct vertex_data));
	sphere->model.vertices_len = vert_count;

	Vec4 north_p = { 0.0f,  radius, 0.0f, 1.0f };
	Vec4 up = { 0.0f, 1.0f, 0.0f, 1.0f };
	Vec4 south_p = { 0.0f, -radius, 0.0f, 1.0f };
	Vec4 down = { 0.0f, -1.0f, 0.0f, 1.0f };

	sphere->model.vertices[0].pos = north_p;
	sphere->model.vertices[0].normal = up;
	sphere->model.vertices[0].material = 0;
	sphere->model.vertices[vert_count - 1].pos = south_p;
	sphere->model.vertices[vert_count - 1].normal = down;
	sphere->model.vertices[vert_count - 1].material = 0;

	float lat_step = M_PI / ( detail + 1 );
	float lon_step = 2 * M_PI / ( detail + 2 );
	int i, j;
	float lat = M_PI / 2 - lat_step;
	for(i = 0; i < par; i++) {
		float lon = 0.0f;
		for(j = 0; j < mer; j++) {
			Vec4 normal = { sinf(lon) * cosf(lat),  sinf(lat), cosf(lon) * cosf(lat), 1.0f };
			sphere->model.vertices[1 + i * mer + j].normal = normal;
			sphere->model.vertices[1 + i * mer + j].pos = vec4_scale(normal, radius);
			sphere->model.vertices[1 + i * mer + j].material = material;
			lon += lon_step;
		}
		lat -= lat_step;
	}

	sphere->model.triangles = 2 * mer + 2 * (par - 1) * mer;

	sphere->model.indices = (uint32_t *)malloc(sphere->model.triangles * 3 * sizeof(uint32_t));

	uint32_t ind = 0;

	for(j = 0; j < mer; j++) {
		sphere->model.indices[ind++] = 0; // north pole
		sphere->model.indices[ind++] = 1 + j;
		sphere->model.indices[ind++] = 1 + ((j + 1) % mer);
	}
	for(i = 0; i < par - 1; i++) {
		for(j = 0; j < mer; j++) {
			sphere->model.indices[ind++] = 1 + i * mer + j;
			sphere->model.indices[ind++] = 1 + (i + 1) * mer + j;
			sphere->model.indices[ind++] = 1 + (i + 1) * mer + ((j + 1) % mer);

			sphere->model.indices[ind++] = 1 + i * mer + j;
			sphere->model.indices[ind++] = 1 + (i + 1) * mer + ((j + 1) % mer);
			sphere->model.indices[ind++] = 1 + i * mer + ((j + 1) % mer);
		}
	}
	for(j = 0; j < mer; j++) {
		sphere->model.indices[ind++] = vert_count - 1; // south pole;
		sphere->model.indices[ind++] = 1 + i * mer + ((j + 1) % mer);
		sphere->model.indices[ind++] = 1 + i * mer + j;
	}
	sphere->model.indices_len = ind;

	return (struct model *)sphere;
}

const struct model_type sphere_model_type = {
	.name = "Sphere",
};

