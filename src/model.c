#include "model.h"

#include <malloc.h>
#include <string.h>

void destroy_model(struct model * model) {

	if (model->vertices) free(model->vertices);
	if (model->indices) free(model->indices);
}

static void normal(vec4 result, const vec4 a, const vec4 b, const vec4 c) {

	vec4 U, V;

	U[0] = b[0] - a[0];
	U[1] = b[1] - a[1];
	U[2] = b[2] - a[2];

	V[0] = c[0] - a[0];
	V[1] = c[1] - a[1];
	V[2] = c[2] - a[2];

	result[0] = U[1]*V[2] - U[2]*V[1];
	result[1] = U[2]*V[0] - U[0]*V[2];
	result[2] = U[0]*V[1] - U[1]*V[0];
	result[3] = 0;
}

void model_compute_normals(struct model * model) {

	uint32_t i;

	struct vertex_data * verts = model->vertices;

	for(i = 0; i < model->vertices_len; i += 3) {
		vec4 t_normal;
		normal(t_normal, verts[i].pos, verts[i + 1].pos, verts[i + 2].pos);
		vec4_norm(verts[i].normal, t_normal);
		verts[i].normal[3] = 1.0f;
		memcpy(verts[i + 1].normal, verts[i].normal, sizeof(vec4));
		memcpy(verts[i + 2].normal, verts[i].normal, sizeof(vec4));
	}
}
