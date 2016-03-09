#ifndef model_h
#define model_h

#include "linalg.h"
#include <stdint.h>

struct vertex_data {
	Vec4 pos;
	Vec4 normal;
	uint32_t material, a, b, c;
};

struct model;

struct model_type {
	const char * name;

	void (*finalize)(struct model * model);
};

struct model {
	const struct model_type * type;

	/* vertex positions and normals */
	struct vertex_data * vertices;
	uint32_t vertices_len;

	/* for indexed draws (when same vertex is to be used multiple times) */
	uint32_t * indices;
	uint32_t indices_len;

	/* total number of triangles to draw */
	uint32_t triangles;
};

void destroy_model(struct model * model);

// helper functions for models

// compute normals for flat surfaces
void model_compute_normals(struct model * model);

#endif
