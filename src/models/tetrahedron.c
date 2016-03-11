
#include "models/tetrahedron.h"

#include <malloc.h>
#include <string.h>

/* tetrahedron vertices */
#define V1 { 1.0f,  1.0f,  1.0f, 1.0f}
#define V2 {-1.0f, -1.0f,  1.0f, 1.0f}
#define V3 {-1.0f,  1.0f, -1.0f, 1.0f}
#define V4 { 1.0f, -1.0f, -1.0f, 1.0f}

#define NULLVEC {0, 0, 0, 0}

#define TRIANGLE_COUNT 4
#define VERT_COUNT TRIANGLE_COUNT * 3

static const struct vertex_data vertices[VERT_COUNT] = {
	{ V1, NULLVEC, 0, V_FLAG_FLAT }, { V2, NULLVEC, 0, V_FLAG_FLAT }, {V4, NULLVEC, 0, V_FLAG_FLAT },
	{ V1, NULLVEC, 0, V_FLAG_FLAT }, { V3, NULLVEC, 0, V_FLAG_FLAT }, {V2, NULLVEC, 0, V_FLAG_FLAT },
	{ V1, NULLVEC, 0, V_FLAG_FLAT }, { V4, NULLVEC, 0, V_FLAG_FLAT }, {V3, NULLVEC, 0, V_FLAG_FLAT },
	{ V2, NULLVEC, 0, V_FLAG_FLAT }, { V3, NULLVEC, 0, V_FLAG_FLAT }, {V4, NULLVEC, 0, V_FLAG_FLAT },
};

struct model * create_tetrahedron(uint32_t material) {

	uint32_t i;

	struct model * model = (struct model *)calloc(1, sizeof(struct model));
	model->type = TETRAHEDRON_MODEL;
	model->vertices = (struct vertex_data *)calloc(VERT_COUNT, sizeof(struct vertex_data));
	model->vertices_len = VERT_COUNT;
	memcpy(model->vertices, vertices, VERT_COUNT * sizeof(struct vertex_data));
	model_compute_normals(model);

	for(i = 0; i < VERT_COUNT; i++) {
		model->vertices[i].material = material;
	}

	model->vertices_len = VERT_COUNT;
	model->triangles = TRIANGLE_COUNT;

	return model;
}

const struct model_type tetrahedron_model_type = {
	.name = "Tetrahedron",
};
