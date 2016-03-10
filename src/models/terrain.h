#ifndef models_terrain_h
#define models_terrain_h

#include "model.h"

struct model * create_terrain(uint32_t width, uint32_t depth, const char * heightmap_path, float sea_level);

float sample_terrain_height(struct model * terrain, float x, float z);

extern const struct model_type terrain_model_type;

#define TERRAIN_MODEL (&terrain_model_type)

#endif
