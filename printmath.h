#ifndef printmath_h
#define printmath_h

#define print_vec3(suffix, vec) printf("%s (%5.1f, %5.1f, %5.1f)\n", \
				(suffix), (vec)[0], (vec)[1], (vec)[2]);
#define print_vec4(suffix, vec) printf("%s (%5.1f, %5.1f, %5.1f, %5.1f)\n", \
	       	(suffix), (vec)[0], (vec)[1], (vec)[2], (vec)[3]);
#define print_mat4x4(suffix, mat) printf("%s\n  " \
		"  [(%5.1f, %5.1f, %5.1f, %5.1f)\n" \
		"   (%5.1f, %5.1f, %5.1f, %5.1f)\n" \
		"   (%5.1f, %5.1f, %5.1f, %5.1f)\n" \
		"   (%5.1f, %5.1f, %5.1f, %5.1f)]\n" \
		, (suffix),\
	       	(mat[0])[0], (mat[0])[1], (mat[0])[2], (mat[0])[3], \
	       	(mat[1])[0], (mat[1])[1], (mat[1])[2], (mat[1])[3], \
	       	(mat[2])[0], (mat[2])[1], (mat[2])[2], (mat[2])[3], \
	       	(mat[3])[0], (mat[3])[1], (mat[3])[2], (mat[3])[3]); \

#endif
