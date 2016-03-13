#ifndef models_terrain_h
#define models_terrain_h

#include "model.h"

struct model * create_terrain(uint32_t width, uint32_t depth, const char * heightmap_path, float sea_level);

float sample_terrain_height(struct model * terrain, float x, float z);

extern const struct model_type terrain_model_type;

#define TERRAIN_MODEL (&terrain_model_type)

#define TERR_WATER_THRESHOLD 41
#define TERR_GRASS_THRESHOLD 43
#define TERR_ROCK_THRESHOLD 150
#define TERR_SNOW_THRESHOLD 180

#define TERR_X_STEP 2.0f
#define TERR_Y_STEP 1.0f
#define TERR_Z_STEP 2.0f

#endif
