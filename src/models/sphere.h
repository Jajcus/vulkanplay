#ifndef models_sphere_h
#define models_sphere_h

#include "model.h"

struct model * create_sphere(uint32_t material, int detail, float radius);

extern const struct model_type sphere_model_type;

#define SPHERE_MODEL (&sphere_model_type)

#endif
