
#include "models/terrain.h"

#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

struct terrain_model {
	struct model model;

	uint32_t width, depth;
	float heightmap[];
};

static const float x_step = 2.0f, y_step = 1.0f, z_step = 2.0f;

struct model * create_terrain(uint32_t width, uint32_t depth, const char * heightmap_path, float sea_level) {

	unsigned char buf[4096];
	FILE * fp;

	fp = fopen(heightmap_path, "rb");
	if (!fp) {
		perror(heightmap_path);
		return NULL;
	}

	struct terrain_model * terrain = (struct terrain_model *)calloc(1,
				sizeof(struct terrain_model) + sizeof(float) * width * depth);
	terrain->model.type = TERRAIN_MODEL;
	terrain->width = width;
	terrain->depth = depth;
	int vert_count = width * depth;
	float * heightmap = terrain->heightmap;

	uint32_t b_read = 0;
	while(b_read < vert_count) {
		uint32_t nbytes = fread(buf, sizeof(unsigned char), 4096, fp);
		if (nbytes < 4096 && nbytes < vert_count - b_read) {
			if (ferror(fp)) {
				perror(heightmap_path);
			}
			else {
				fprintf(stderr, "%s - unexpected EOF", heightmap_path);
			}
			fclose(fp);
			free(terrain);
			return NULL;
		}
		uint32_t i;
		for(i = 0; i < nbytes; i++) {
			heightmap[b_read + i] = (float)buf[i] - sea_level;
		}
		b_read += nbytes;
	}

	struct vertex_data * verts = (struct vertex_data *)calloc(vert_count, sizeof(struct vertex_data));
	terrain->model.vertices = verts;
	terrain->model.vertices_len = vert_count;
	terrain->model.triangles = (width - 1) * (depth - 1) * 2;
	uint32_t * indices = (uint32_t *)malloc(terrain->model.triangles * 3 * sizeof(uint32_t));
	terrain->model.indices = indices;

	int i, j;

	float z = - z_step * width / 2;

	int v = 0;
	uint32_t ind = 0;
	for(i = 0; i < depth; i ++) {
		float x = - x_step * width / 2;
		for(j = 0; j < width; j++) {
			float y = heightmap[(depth - i - 1) * width + j] * y_step;
			Vec4 pos = { x, y, z, 1.0f };
			Vec4 normal = { 0.0f, 1.0f, 0.0f, 0.0f };
			verts[v].pos = pos;
			verts[v].material = 2;
			if (i > 0 && j > 0) {
				int v2 = v - width, v3 = v - width - 1, v4 = v - 1;
				/*    v4----v
				 *    | 2 / |
				 *    | / 1 |
				 *    v3----v2   */

				Vec4 n1 = triangle_normal(verts[v].pos, verts[v2].pos, verts[v3].pos);
				Vec4 n2 = triangle_normal(verts[v].pos, verts[v3].pos, verts[v4].pos);

				normal = vec4_norm(vec4_add(n1, n2));

				verts[v2].normal = vec4_norm(vec4_add(verts[v2].normal, normal));
				verts[v3].normal = vec4_norm(vec4_add(verts[v3].normal, normal));
				verts[v4].normal = vec4_norm(vec4_add(verts[v4].normal, normal));

				indices[ind++] = v;
				indices[ind++] = v2;
				indices[ind++] = v3;
				indices[ind++] = v;
				indices[ind++] = v3;
				indices[ind++] = v4;
			}
			verts[v].normal = normal;
			v ++;
			x += x_step;
		}
		z += z_step;
	}
	terrain->model.indices_len = ind;

	return (struct model *)terrain;
}

float sample_terrain_height(struct model * model, float x, float z) {

	assert(model->type == TERRAIN_MODEL);
	struct terrain_model * terrain = (struct terrain_model *)model;

	float sx = x / x_step + terrain->width / 2 + 0.5;
	float sz = z / z_step + terrain->depth / 2 + 0.5;

	if (sx < 0) sx = 0;
	uint32_t nx = sx;
	if (sx >= terrain->width) sx = terrain->width - 1;
	if (sz < 0) sz = 0;
	uint32_t nz = sz;
	if (sz >= terrain->depth) sz = terrain->depth - 1;
	sz = terrain->depth - sz - 1;
	return terrain->heightmap[terrain->width * nz + nx];
}


void terrain_finalize(struct model * model) {

	assert(model->type == TERRAIN_MODEL);
}

const struct model_type terrain_model_type = {
	.name = "Terrain",
	.finalize = terrain_finalize,
};

