
#define vec3_add lm_vec3_add
#define vec3_len lm_vec3_len
#define vec3_mul_cross lm_vec3_mul_cross
#define vec3_mul_inner lm_vec3_mul_inner
#define vec3_mul_outer lm_vec3_mul_outer
#define vec3_norm lm_vec3_norm
#define vec3_reflect lm_vec3_reflect
#define vec3_scale lm_vec3_scale
#define vec3_sub lm_vec3_sub
#define vec4_add lm_vec4_add
#define vec4_len lm_vec4_len
#define vec4_mul_cross lm_vec4_mul_cross
#define vec4_mul_inner lm_vec4_mul_inner
#define vec4_norm lm_vec4_norm
#define vec4_reflect lm_vec4_reflect
#define vec4_scale lm_vec4_scale
#define vec4_sub lm_vec4_sub


#include "linmath.h"

#undef vec3_add
#undef vec3_len
#undef vec3_mul_cross
#undef vec3_mul_inner
#undef vec3_mul_outer
#undef vec3_norm
#undef vec3_reflect
#undef vec3_scale
#undef vec3_sub
#undef vec4_add
#undef vec4_len
#undef vec4_mul_cross
#undef vec4_mul_inner
#undef vec4_norm
#undef vec4_reflect
#undef vec4_scale
#undef vec4_sub


#include "linalg.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define epsilon (0.00000001f)

#define CHECK(r, R, msg) if (memcmp(r, &R, sizeof(R))) { \
	fprintf(stderr, "FAIL: %s\n", msg); abort(); }

#define CHECK_F(r, R, msg) if ( fabs(r-R) > epsilon ) { \
	fprintf(stderr, "FAIL: %s\n", msg); abort(); }

void test_vec3(void) {

	vec3 a = {1, 2, 3};
	Vec3 A = {1, 2, 3};
	vec3 b = {3, 4, 5};
	Vec3 B = {3, 4, 5};
	float f = 0.5;

	vec3 r;
	Vec3 R;
	float fr;
	float FR;

	lm_vec3_add(r, a, b);
	R = vec3_add(A, B);
	CHECK(r, R, "vec3_add");

	memset(r, 0, sizeof(vec3));
	memset(&R, 0, sizeof(Vec3));
	lm_vec3_sub(r, a, b);
	R = vec3_sub(A, B);
	CHECK(r, R, "vec3_sub");

	memset(r, 0, sizeof(vec3));
	memset(&R, 0, sizeof(Vec3));
	lm_vec3_scale(r, a, f);
	R = vec3_scale(A, f);
	CHECK(r, R, "vec3_scale");

	memset(r, 0, sizeof(vec3));
	memset(&R, 0, sizeof(Vec3));
	fr = lm_vec3_mul_inner(a, b);
	FR = vec3_mul_inner(A, B);
	CHECK_F(fr, FR, "vec3_mul_inner");

	memset(r, 0, sizeof(vec3));
	memset(&R, 0, sizeof(Vec3));
	lm_vec3_mul_cross(r, a, b);
	R = vec3_mul_cross(A, B);
	CHECK(r, R, "vec3_mul_cross");

	memset(r, 0, sizeof(vec3));
	memset(&R, 0, sizeof(Vec3));
	fr = lm_vec3_len(a);
	FR = vec3_len(A);
	CHECK_F(fr, FR, "vec3_len");

	memset(r, 0, sizeof(vec3));
	memset(&R, 0, sizeof(Vec3));
	lm_vec3_norm(r, a);
	R = vec3_norm(A);
	CHECK(r, R, "vec3_norm");

	memset(r, 0, sizeof(vec3));
	memset(&R, 0, sizeof(Vec3));
	lm_vec3_reflect(r, a, b);
	R = vec3_reflect(A, B);
	CHECK(r, R, "vec3_reflect");
}

void test_vec4(void) {

	vec4 a = {1, 2, 3, 4};
	Vec4 A = {1, 2, 3, 4};
	vec4 b = {3, 4, 5, 6};
	Vec4 B = {3, 4, 5, 6};
	float f = 0.5;

	vec4 r;
	Vec4 R;
	float fr;
	float FR;

	lm_vec4_add(r, a, b);
	R = vec4_add(A, B);
	CHECK(r, R, "vec4_add");

	memset(r, 0, sizeof(vec4));
	memset(&R, 0, sizeof(Vec4));
	lm_vec4_sub(r, a, b);
	R = vec4_sub(A, B);
	CHECK(r, R, "vec4_sub");

	memset(r, 0, sizeof(vec4));
	memset(&R, 0, sizeof(Vec4));
	lm_vec4_scale(r, a, f);
	R = vec4_scale(A, f);
	CHECK(r, R, "vec4_scale");

	memset(r, 0, sizeof(vec4));
	memset(&R, 0, sizeof(Vec4));
	fr = lm_vec4_mul_inner(a, b);
	FR = vec4_mul_inner(A, B);
	CHECK_F(fr, FR, "vec4_mul_inner");

	memset(r, 0, sizeof(vec4));
	memset(&R, 0, sizeof(Vec4));
	lm_vec4_mul_cross(r, a, b);
	R = vec4_mul_cross(A, B);
	CHECK(r, R, "vec4_mul_cross");

	memset(r, 0, sizeof(vec4));
	memset(&R, 0, sizeof(Vec4));
	fr = lm_vec4_len(a);
	FR = vec4_len(A);
	CHECK_F(fr, FR, "vec4_len");

	memset(r, 0, sizeof(vec4));
	memset(&R, 0, sizeof(Vec4));
	lm_vec4_norm(r, a);
	R = vec4_norm(A);
	CHECK(r, R, "vec4_norm");

	memset(r, 0, sizeof(vec4));
	memset(&R, 0, sizeof(Vec4));
	lm_vec4_reflect(r, a, b);
	R = vec4_reflect(A, B);
	CHECK(r, R, "vec4_reflect");
}

void test_mat4(void) {

	mat4x4 a = {
		{1, 2, 3, 4},
		{4, 3, 2, 1},
		{2, 1, 3, 4},
		{4, 3, 1, 2},
	};
	Mat4 A = {{
		{1, 2, 3, 4},
		{4, 3, 2, 1},
		{2, 1, 3, 4},
		{4, 3, 1, 2},
	}};
	mat4x4 b = {
		{1, 0, 1, 0},
		{2, 3, 3, 1},
		{2, 1, 3, 1},
		{1, 3, 1, 2},
	};
	Mat4 B = {{
		{1, 0, 1, 0},
		{2, 3, 3, 1},
		{2, 1, 3, 1},
		{1, 3, 1, 2},
	}};

	float f = 0.5;

	vec4 v = {1, 2, 3, 4};
	Vec4 V = {1, 2, 3, 4};
	vec3 v3 = {1, 2, 3};
	Vec3 V3 = {1, 2, 3};
	vec3 w3 = {3, 4, 5};
	Vec3 W3 = {3, 4, 5};
	vec3 u3 = {1, 0, 0};
	Vec3 U3 = {1, 0, 0};

	mat4x4 r;
	Mat4 R;
	vec4 vr;
	Vec4 VR;

	memset(r, 0, sizeof(mat4x4));
	memset(&R, 0, sizeof(Mat4));
	mat4x4_identity(r);
	R = MAT4_IDENTITY;
	CHECK(r, R, "MAT4_IDENTITY");

	memset(r, 0, sizeof(mat4x4));
	memset(&R, 0, sizeof(Mat4));
	mat4x4_transpose(r, a);
	R = mat4_transpose(A);
	CHECK(r, R, "mat4_transpose");

	memset(r, 0, sizeof(mat4x4));
	memset(&R, 0, sizeof(Mat4));
	mat4x4_add(r, a, b);
	R = mat4_add(A, B);
	CHECK(r, R, "mat4_add");

	memset(r, 0, sizeof(mat4x4));
	memset(&R, 0, sizeof(Mat4));
	mat4x4_sub(r, a, b);
	R = mat4_sub(A, B);
	CHECK(r, R, "mat4_sub");

	memset(r, 0, sizeof(mat4x4));
	memset(&R, 0, sizeof(Mat4));
	mat4x4_scale(r, a, f);
	R = mat4_scale(A, f);
	CHECK(r, R, "mat4_scale");

	memset(r, 0, sizeof(mat4x4));
	memset(&R, 0, sizeof(Mat4));
	mat4x4_mul(r, a, b);
	R = mat4_mul(A, B);
	CHECK(r, R, "mat4_mul");

	memset(r, 0, sizeof(mat4x4));
	memset(&R, 0, sizeof(Mat4));
	mat4x4_scale_aniso(r, a, 1, 2, 3);
	R = mat4_scale_aniso(A, 1, 2, 3);
	CHECK(r, R, "mat4_scale_aniso");

	memset(r, 0, sizeof(mat4x4));
	memset(&R, 0, sizeof(Mat4));
	mat4x4_mul_vec4(vr, a, v);
	VR = mat4_mul_vec4(A, V);
	CHECK(vr, VR, "mat4_mul_vec4");

	memset(r, 0, sizeof(mat4x4));
	memset(&R, 0, sizeof(Mat4));
	mat4x4_translate(r, 1, 2, 3);
	R = mat4_translate(1, 2, 3);
	CHECK(r, R, "mat4_translate");

	memcpy(r, a, sizeof(mat4x4));
	memcpy(&R, &A, sizeof(Mat4));
	mat4x4_translate_in_place(r, 1, 2, 3);
	mat4_translate_in_place(&R, 1, 2, 3);
	CHECK(r, R, "mat4_translate_in_place");

	memset(r, 0, sizeof(mat4x4));
	memset(&R, 0, sizeof(Mat4));
	mat4x4_from_vec3_mul_outer(r, v3, w3);
	R = mat4_from_vec3_mul_outer(V3, W3);
	CHECK(r, R, "mat4_from_vec3_mul_outer");

	memset(r, 0, sizeof(mat4x4));
	memset(&R, 0, sizeof(Mat4));
	mat4x4_rotate(r, a, 1, 2, 3, 1);
	R = mat4_rotate(A, 1, 2, 3, 1);
	CHECK(r, R, "mat4_rotate");

	memset(r, 0, sizeof(mat4x4));
	memset(&R, 0, sizeof(Mat4));
	mat4x4_rotate_X(r, a, 1);
	R = mat4_rotate_X(A, 1);
	CHECK(r, R, "mat4_rotate_X");

	memset(r, 0, sizeof(mat4x4));
	memset(&R, 0, sizeof(Mat4));
	mat4x4_rotate_Y(r, a, 1);
	R = mat4_rotate_Y(A, 1);
	CHECK(r, R, "mat4_rotate_Y");

	memset(r, 0, sizeof(mat4x4));
	memset(&R, 0, sizeof(Mat4));
	mat4x4_rotate_Z(r, a, 1);
	R = mat4_rotate_Z(A, 1);
	CHECK(r, R, "mat4_rotate_Z");

	memset(r, 0, sizeof(mat4x4));
	memset(&R, 0, sizeof(Mat4));
	mat4x4_invert(r, a);
	R = mat4_invert(A);
	CHECK(r, R, "mat4_invert");

	memset(r, 0, sizeof(mat4x4));
	memset(&R, 0, sizeof(Mat4));
	mat4x4_orthonormalize(r, a);
	R = mat4_orthonormalize(A);
	CHECK(r, R, "mat4_orthonormalize");

	memset(r, 0, sizeof(mat4x4));
	memset(&R, 0, sizeof(Mat4));
	mat4x4_frustum(r, 1, 2, 3, 4, 5, 6);
	R = mat4_frustum(1, 2, 3, 4, 5, 6);
	CHECK(r, R, "mat4_frustum");

	memset(r, 0, sizeof(mat4x4));
	memset(&R, 0, sizeof(Mat4));
	mat4x4_ortho(r, 1, 2, 3, 4, 5, 6);
	R = mat4_ortho(1, 2, 3, 4, 5, 6);
	CHECK(r, R, "mat4_ortho");

	memset(r, 0, sizeof(mat4x4));
	memset(&R, 0, sizeof(Mat4));
	mat4x4_perspective(r, 1, 2, 3, 4);
	R = mat4_perspective(1, 2, 3, 4);
	CHECK(r, R, "mat4_perspective");

	memset(r, 0, sizeof(mat4x4));
	memset(&R, 0, sizeof(Mat4));
	mat4x4_look_at(r, v3, w3, u3);
	R = mat4_look_at(V3, W3, U3);
	CHECK(r, R, "mat4_look_at");
}

int main(int argc, char ** argv) {

	printf("Testing linalg.h...\n");

	if (sizeof(Vec3) != 3 * sizeof(float)) {
		printf("invalid size of Vec3 (%lu instead of %lu)\n",
				(unsigned long)sizeof(Vec3), (unsigned long)3 * sizeof(float));
		exit(1);
	}
	if (sizeof(Vec4) != 4 * sizeof(float)) {
		printf("invalid size of Vec3 (%lu instead of %lu)\n",
				(unsigned long)sizeof(Vec4), (unsigned long)4 * sizeof(float));
		exit(1);
	}
	if (sizeof(Mat4) != 16 * sizeof(float)) {
		printf("invalid size of Mat4 (%lu instead of %lu)\n",
				(unsigned long)sizeof(Mat4), (unsigned long)16 * sizeof(float));
		exit(1);
	}

	if (!linalg_sanity_ok()) {
		fprintf(stderr, "linmath.h structure sanity check failed!\n");
		abort();
	}

	test_vec3();
	test_vec4();
	test_mat4();

	printf("passed!\n");
}
