#ifndef models_plane_h
#define models_plane_h

#include "model.h"

struct model * create_plane(uint32_t material);

extern const struct model_type plane_model_type;

#define PLANE_MODEL (&plane_model_type)

#endif
