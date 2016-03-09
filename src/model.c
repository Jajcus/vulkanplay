#include "model.h"

#include <malloc.h>
#include <string.h>

void destroy_model(struct model * model) {

	if (model->type && model->type->finalize) {
		model->type->finalize(model);
	}
	if (model->vertices) free(model->vertices);
	if (model->indices) free(model->indices);
}

static Vec4 normal(Vec4 a, Vec4 b, Vec4 c) {

	Vec4 result;
	Vec4 U, V;

	U.x = b.x - a.x;
	U.y = b.y - a.y;
	U.z = b.z - a.z;

	V.x = c.x - a.x;
	V.y = c.y - a.y;
	V.z = c.z - a.z;

	result.x = U.y*V.z - U.z*V.y;
	result.y = U.z*V.x - U.x*V.z;
	result.z = U.x*V.y - U.y*V.x;
	result.w = 0;

	return result;
}

void model_compute_normals(struct model * model) {

	uint32_t i;

	struct vertex_data * verts = model->vertices;

	for(i = 0; i < model->vertices_len; i += 3) {
		Vec4 t_normal = normal(verts[i].pos, verts[i + 1].pos, verts[i + 2].pos);
		verts[i].normal = vec4_norm(t_normal);
		verts[i].normal.w = 1.0f;
		verts[i + 1].normal = verts[i].normal;
		verts[i + 2].normal = verts[i].normal;
	}
}
