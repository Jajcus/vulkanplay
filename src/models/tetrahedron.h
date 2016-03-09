#ifndef models_tetrahedron_h
#define models_tetrahedron_h

#include "model.h"

struct model * create_tetrahedron(uint32_t material);

extern const struct model_type tetrahedron_model_type;

#define TETRAHEDRON_MODEL (&tetrahedron_model_type)

#endif
