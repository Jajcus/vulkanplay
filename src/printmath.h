#ifndef printmath_h
#define printmath_h

#define print_vec3(suffix, vec) printf("%s (%5.1f, %5.1f, %5.1f)\n", \
				(suffix), (vec).x, (vec).y, (vec).z);
#define print_vec4(suffix, vec) printf("%s (%5.1f, %5.1f, %5.1f, %5.1f)\n", \
		(suffix), (vec).x, (vec).y, (vec).z, (vec).w);
#define print_mat4x4(suffix, mat) printf("%s\n  " \
		"  [(%5.1f, %5.1f, %5.1f, %5.1f)\n" \
		"   (%5.1f, %5.1f, %5.1f, %5.1f)\n" \
		"   (%5.1f, %5.1f, %5.1f, %5.1f)\n" \
		"   (%5.1f, %5.1f, %5.1f, %5.1f)]\n" \
		, (suffix),\
		mat.c[0].x, mat.c[0].y, mat.c[0].z, mat.c[0].w, \
		mat.c[1].x, mat.c[1].y, mat.c[1].z, mat.c[1].w, \
		mat.c[2].x, mat.c[2].y, mat.c[2].z, mat.c[2].w, \
		mat.c[3].x, mat.c[3].y, mat.c[3].z, mat.c[3].w); \

#endif
