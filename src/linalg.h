#ifndef linalg_h
#define linalg_h

#include <math.h>

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

/* angle convertions */
#define deg_to_rad(angle) ((angle) * M_PI / 180.0)
#define rad_to_deg(angle) ((angle) * 180.0 / M_PI)

typedef struct vec3 {
	float x;
	float y;
	float z;
} Vec3;

typedef struct vec4 {
	float x;
	float y;
	float z;
	float w;
} Vec4;

typedef struct mat4 {
	Vec4 c[4];
} Mat4;

typedef union vec34 {
	Vec4 v4;
	Vec3 v3;
} Vec34;

static inline Vec3 vec3_add(const Vec3 a, const Vec3 b) {
	Vec3 r;
	r.x = a.x + b.x;
	r.y = a.y + b.y;
	r.z = a.z + b.z;
	return r;
}
static inline Vec3 vec3_sub(const Vec3 a, const Vec3 b) {
	Vec3 r;
	r.x = a.x - b.x;
	r.y = a.y - b.y;
	r.z = a.z - b.z;
	return r;
}
static inline Vec3 vec3_scale(const Vec3 a, float s) {
	Vec3 r;
	r.x = a.x * s;
	r.y = a.y * s;
	r.z = a.z * s;
	return r;
}
static inline float vec3_mul_inner(const Vec3 a, const Vec3 b) {
	float r = 0.0f;
	r += a.x * b.x;
	r += a.y * b.y;
	r += a.z * b.z;
	return r;
}
static inline Vec3 vec3_mul_cross(const Vec3 a, const Vec3 b) {
	Vec3 r;
	r.x = a.y * b.z - a.z * b.y;
	r.y = a.z * b.x - a.x * b.z;
	r.z = a.x * b.y - a.y * b.x;
  	return r;
}
static inline float vec3_len(const Vec3 v) {
	return sqrtf(vec3_mul_inner(v, v));
}
static inline Vec3 vec3_norm(const Vec3 v) {
	float k = 1.f / vec3_len(v);
	return vec3_scale(v, k);
}
static inline Vec3 vec3_reflect(const Vec3 v, const Vec3 n) {
	Vec3 r;
	float p = 2.f * vec3_mul_inner(v, n);
	r.x = v.x - p * n.x;
	r.y = v.y - p * n.y;
	r.z = v.z - p * n.z;
	return r;
}

static inline Vec4 vec4_add(const Vec4 a, const Vec4 b) {
	Vec4 r;
	r.x = a.x + b.x;
	r.y = a.y + b.y;
	r.z = a.z + b.z;
	r.w = a.w + b.w;
	return r;
}
static inline Vec4 vec4_sub(const Vec4 a, const Vec4 b) {
	Vec4 r;
	r.x = a.x - b.x;
	r.y = a.y - b.y;
	r.z = a.z - b.z;
	r.w = a.w - b.w;
	return r;
}
static inline Vec4 vec4_scale(const Vec4 a, float s) {
	Vec4 r;
	r.x = a.x * s;
	r.y = a.y * s;
	r.z = a.z * s;
	r.w = a.w * s;
	return r;
}
static inline float vec4_mul_inner(const Vec4 a, const Vec4 b) {
	float r = 0.0f;
	r += a.x * b.x;
	r += a.y * b.y;
	r += a.z * b.z;
	r += a.w * b.w;
	return r;
}
static inline Vec4 vec4_mul_cross(const Vec4 a, const Vec4 b) {

	Vec4 r;
	r.x = a.y * b.z - a.z * b.y;
	r.y = a.z * b.x - a.x * b.z;
	r.z = a.x * b.y - a.y * b.x;
	r.w = 1.0f;
	return r;
}
static inline float vec4_len(const Vec4 v) {
	return sqrtf(vec4_mul_inner(v, v));
}
static inline Vec4 vec4_norm(const Vec4 v) {
	float k = 1.f / vec4_len(v);
	return vec4_scale(v, k);
}
static inline Vec4 vec4_reflect(const Vec4 v, const Vec4 n) {
	Vec4 r;
	float p = 2.f * vec4_mul_inner(v, n);
	r.x = v.x - p * n.x;
	r.y = v.y - p * n.y;
	r.z = v.z - p * n.z;
	r.w = v.w - p * n.w;
	return r;
}

static const Mat4 MAT4_IDENTITY = {{
	{ 1, 0, 0, 0 },
	{ 0, 1, 0, 0 },
	{ 0, 0, 1, 0 },
	{ 0, 0, 0, 1 }}};

static inline Mat4 mat4_transpose(const Mat4 n) {
	Mat4 m;
	const float * N = (const float *)&n;
	float * M = (float *)&m;

	int i, j;
	for (j = 0; j < 4; ++j)
		for (i = 0; i < 4; ++i)
			M[i * 4 + j] = N[j * 4 + i];
	return m;
}
static inline Mat4 mat4_add(const Mat4 a, const Mat4 b) {
	Mat4 m;
	int i;
	for (i = 0; i < 4; ++i)
		m.c[i] = vec4_add(a.c[i], b.c[i]);
	return m;
}
static inline Mat4 mat4_sub(const Mat4 a, const Mat4 b) {
	Mat4 m;
	int i;
	for (i = 0; i < 4; ++i)
		m.c[i] = vec4_sub(a.c[i], b.c[i]);
	return m;
}
static inline Mat4 mat4_scale(const Mat4 a, float k) {
	Mat4 m;
	int i;
	for (i = 0; i < 4; ++i)
		m.c[i] = vec4_scale(a.c[i], k);
	return m;
}
static inline Mat4 mat4_scale_aniso(const Mat4 a, float x, float y, float z) {
	Mat4 m;
	m.c[0] = vec4_scale(a.c[0], x);
	m.c[1] = vec4_scale(a.c[1], y);
	m.c[2] = vec4_scale(a.c[2], z);
	m.c[3] = a.c[3];
	return m;
}
static inline Mat4 mat4_mul(const Mat4 a, const Mat4 b) {
	Mat4 m;
	float * A = (float *)&a;
	float * B = (float *)&b;
	float * M = (float *)&m;
	int k, r, c;
	for (c = 0; c < 4; ++c) {
		for (r = 0; r < 4; ++r) {
			M[c * 4 + r] = 0.f;
			for (k = 0; k < 4; ++k) {
				M[c * 4 + r] += A[k * 4 + r] * B[c * 4 + k];
			}
		}
	}
	return m;
}
static inline Vec4 mat4_mul_vec4(const Mat4 m, const Vec4 v) {
	Vec4 r;
	const float * M = (const float *)&m;
	const float * V = (const float *)&v;
	float * R = (float *)&r;
	int i, j;
	for (j = 0; j < 4; ++j) {
		R[j] = 0.0f;
		for (i = 0; i < 4; ++i) {
			R[j] += M[i * 4 + j] * V[i];
		}
	}
	return r;
}

static inline Mat4 mat4_translate(float x, float y, float z) {
	Mat4 t = MAT4_IDENTITY;

	t.c[3].x = x;
	t.c[3].y = y;
	t.c[3].z = z;
	return t;
}

static inline void mat4_translate_in_place(Mat4 * m, float x, float y, float z) {
	Vec4 t = {x, y, z, 0};

	Vec4 rx = {m->c[0].x, m->c[1].x, m->c[2].x, m->c[3].x};
	m->c[3].x += vec4_mul_inner(rx, t);
	Vec4 ry = {m->c[0].y, m->c[1].y, m->c[2].y, m->c[3].y};
	m->c[3].y += vec4_mul_inner(ry, t);
	Vec4 rz = {m->c[0].z, m->c[1].z, m->c[2].z, m->c[3].z};
	m->c[3].z += vec4_mul_inner(rz, t);
	Vec4 rw = {m->c[0].w, m->c[1].w, m->c[2].w, m->c[3].w};
	m->c[3].w += vec4_mul_inner(rw, t);
}
static inline Mat4 mat4_from_vec3_mul_outer(const Vec3 a, const Vec3 b) {
	Mat4 m;
	float * A = (float *)&a;
	float * B = (float *)&b;
	float * M = (float *)&m;
	int i, j;
	for (i = 0; i < 4; ++i)
		for (j = 0; j < 4; ++j)
			M[i * 4 + j] = i < 3 && j < 3 ? A[i] * B[j] : 0.f;
    	return m;
}
static inline Mat4 mat4_rotate(const Mat4 M, float x, float y, float z, float angle) {

	float s = sinf(angle);
	float c = cosf(angle);
	Vec3 u = {x, y, z};

	if (vec3_len(u) > 1e-4) {
		u = vec3_norm(u);

		Mat4 T = mat4_from_vec3_mul_outer(u, u);

		Mat4 S = {{{0, u.z, -u.y, 0},
			    {-u.z, 0, u.x, 0},
			    {u.y, -u.x, 0, 0},
			    {0, 0, 0, 0}}};

		S = mat4_scale(S, s);

		Mat4 C = mat4_sub(MAT4_IDENTITY, T);

		C = mat4_scale(C, c);

		T = mat4_add(T, C);
		T = mat4_add(T, S);

		T.c[3].w = 1.0;
		return mat4_mul(M, T);
	}
	else {
		return M;
	}
}
static inline Mat4 mat4_rotate_X(const Mat4 M, float angle) {
    float s = sinf(angle);
    float c = cosf(angle);
    Mat4 R = {{{1.f, 0.f, 0.f, 0.f},
                {0.f, c, s, 0.f},
                {0.f, -s, c, 0.f},
                {0.f, 0.f, 0.f, 1.f}}};
    return mat4_mul(M, R);
}
static inline Mat4 mat4_rotate_Y(const Mat4 M, float angle) {
    float s = sinf(angle);
    float c = cosf(angle);
    Mat4 R = {{{c, 0.f, s, 0.f},
                {0.f, 1.f, 0.f, 0.f},
                {-s, 0.f, c, 0.f},
                {0.f, 0.f, 0.f, 1.f}}};
    return mat4_mul(M, R);
}
static inline Mat4 mat4_rotate_Z(const Mat4 M, float angle) {
    float s = sinf(angle);
    float c = cosf(angle);
    Mat4 R = {{{c, s, 0.f, 0.f},
                {-s, c, 0.f, 0.f},
                {0.f, 0.f, 1.f, 0.f},
                {0.f, 0.f, 0.f, 1.f}}};
    return mat4_mul(M, R);
}
static inline Mat4 mat4_invert(const Mat4 m) {
	Mat4 t;
	const float * M = (const float *)&m;
	float * T = (float *)&t;

	float s[6];
	float c[6];
	s[0] = M[0 * 4 + 0] * M[1 * 4 + 1] - M[1 * 4 + 0] * M[0 * 4 + 1];
	s[1] = M[0 * 4 + 0] * M[1 * 4 + 2] - M[1 * 4 + 0] * M[0 * 4 + 2];
	s[2] = M[0 * 4 + 0] * M[1 * 4 + 3] - M[1 * 4 + 0] * M[0 * 4 + 3];
	s[3] = M[0 * 4 + 1] * M[1 * 4 + 2] - M[1 * 4 + 1] * M[0 * 4 + 2];
	s[4] = M[0 * 4 + 1] * M[1 * 4 + 3] - M[1 * 4 + 1] * M[0 * 4 + 3];
	s[5] = M[0 * 4 + 2] * M[1 * 4 + 3] - M[1 * 4 + 2] * M[0 * 4 + 3];

	c[0] = M[2 * 4 + 0] * M[3 * 4 + 1] - M[3 * 4 + 0] * M[2 * 4 + 1];
	c[1] = M[2 * 4 + 0] * M[3 * 4 + 2] - M[3 * 4 + 0] * M[2 * 4 + 2];
	c[2] = M[2 * 4 + 0] * M[3 * 4 + 3] - M[3 * 4 + 0] * M[2 * 4 + 3];
	c[3] = M[2 * 4 + 1] * M[3 * 4 + 2] - M[3 * 4 + 1] * M[2 * 4 + 2];
	c[4] = M[2 * 4 + 1] * M[3 * 4 + 3] - M[3 * 4 + 1] * M[2 * 4 + 3];
	c[5] = M[2 * 4 + 2] * M[3 * 4 + 3] - M[3 * 4 + 2] * M[2 * 4 + 3];

	/* Assumes it is invertible */
	float idet = 1.0f / (s[0] * c[5] - s[1] * c[4] + s[2] * c[3] + s[3] * c[2] -
			 s[4] * c[1] + s[5] * c[0]);

	T[0 * 4 + 0] = (M[1 * 4 + 1] * c[5] - M[1 * 4 + 2] * c[4] + M[1 * 4 + 3] * c[3]) * idet;
	T[0 * 4 + 1] = (-M[0 * 4 + 1] * c[5] + M[0 * 4 + 2] * c[4] - M[0 * 4 + 3] * c[3]) * idet;
	T[0 * 4 + 2] = (M[3 * 4 + 1] * s[5] - M[3 * 4 + 2] * s[4] + M[3 * 4 + 3] * s[3]) * idet;
	T[0 * 4 + 3] = (-M[2 * 4 + 1] * s[5] + M[2 * 4 + 2] * s[4] - M[2 * 4 + 3] * s[3]) * idet;

	T[1 * 4 + 0] = (-M[1 * 4 + 0] * c[5] + M[1 * 4 + 2] * c[2] - M[1 * 4 + 3] * c[1]) * idet;
	T[1 * 4 + 1] = (M[0 * 4 + 0] * c[5] - M[0 * 4 + 2] * c[2] + M[0 * 4 + 3] * c[1]) * idet;
	T[1 * 4 + 2] = (-M[3 * 4 + 0] * s[5] + M[3 * 4 + 2] * s[2] - M[3 * 4 + 3] * s[1]) * idet;
	T[1 * 4 + 3] = (M[2 * 4 + 0] * s[5] - M[2 * 4 + 2] * s[2] + M[2 * 4 + 3] * s[1]) * idet;

	T[2 * 4 + 0] = (M[1 * 4 + 0] * c[4] - M[1 * 4 + 1] * c[2] + M[1 * 4 + 3] * c[0]) * idet;
	T[2 * 4 + 1] = (-M[0 * 4 + 0] * c[4] + M[0 * 4 + 1] * c[2] - M[0 * 4 + 3] * c[0]) * idet;
	T[2 * 4 + 2] = (M[3 * 4 + 0] * s[4] - M[3 * 4 + 1] * s[2] + M[3 * 4 + 3] * s[0]) * idet;
	T[2 * 4 + 3] = (-M[2 * 4 + 0] * s[4] + M[2 * 4 + 1] * s[2] - M[2 * 4 + 3] * s[0]) * idet;

	T[3 * 4 + 0] = (-M[1 * 4 + 0] * c[3] + M[1 * 4 + 1] * c[1] - M[1 * 4 + 2] * c[0]) * idet;
	T[3 * 4 + 1] = (M[0 * 4 + 0] * c[3] - M[0 * 4 + 1] * c[1] + M[0 * 4 + 2] * c[0]) * idet;
	T[3 * 4 + 2] = (-M[3 * 4 + 0] * s[3] + M[3 * 4 + 1] * s[1] - M[3 * 4 + 2] * s[0]) * idet;
	T[3 * 4 + 3] = (M[2 * 4 + 0] * s[3] - M[2 * 4 + 1] * s[1] + M[2 * 4 + 2] * s[0]) * idet;

	return t;
}
static inline Mat4 mat4_orthonormalize(const Mat4 m) {
	Mat4 r = m;
	float s = 1.0;
	Vec3 h;

	Vec3 * cols[] = {
		(Vec3 *)&r.c[0],
		(Vec3 *)&r.c[1],
		(Vec3 *)&r.c[2],
		(Vec3 *)&r.c[3],
	};

	*cols[2] = vec3_norm(*cols[2]);

	s = vec3_mul_inner(*cols[1], *cols[2]);
	h = vec3_scale(*cols[2], s);
	*cols[1] = vec3_sub(*cols[1], h);
	*cols[2] = vec3_norm(*cols[2]);

	s = vec3_mul_inner(*cols[1], *cols[2]);
	h = vec3_scale(*cols[2], s);
	*cols[1] = vec3_sub(*cols[1], h);
	*cols[1] = vec3_norm(*cols[1]);

	s = vec3_mul_inner(*cols[0], *cols[1]);
	h = vec3_scale(*cols[1], s);
	*cols[0] = vec3_sub(*cols[0], h);
	*cols[0] = vec3_norm(*cols[0]);

	return r;
}

static inline Mat4 mat4_frustum(float l, float r, float b, float t, float n, float f) {
	Mat4 m;

	m.c[0].x = 2.f * n / (r - l);
	m.c[0].y = m.c[0].z = m.c[0].w = 0.f;

	m.c[1].y = 2.f * n / (t - b);
	m.c[1].x = m.c[1].z = m.c[1].w = 0.f;

	m.c[2].x = (r + l) / (r - l);
	m.c[2].y = (t + b) / (t - b);
	m.c[2].z = -(f + n) / (f - n);
	m.c[2].w = -1.f;

	m.c[3].z = -2.f * (f * n) / (f - n);
	m.c[3].x = m.c[3].y = m.c[3].w = 0.f;

	return m;
}
static inline Mat4 mat4_ortho(float l, float r, float b, float t, float n, float f) {
	Mat4 m;
	m.c[0].x = 2.f / (r - l);
	m.c[0].y = m.c[0].z = m.c[0].w = 0.f;

	m.c[1].y = 2.f / (t - b);
	m.c[1].x = m.c[1].z = m.c[1].w = 0.f;

	m.c[2].z = -2.f / (f - n);
	m.c[2].x = m.c[2].y = m.c[2].w = 0.f;

	m.c[3].x = -(r + l) / (r - l);
	m.c[3].y = -(t + b) / (t - b);
	m.c[3].z = -(f + n) / (f - n);
	m.c[3].w = 1.f;
	return m;
}
static inline Mat4 mat4_perspective(float y_fov, float aspect, float n, float f) {
	Mat4 m;
	float const a = (float)(1.f / tan(y_fov / 2.f));

	m.c[0].x = a / aspect;
	m.c[0].y = 0.f;
	m.c[0].z = 0.f;
	m.c[0].w = 0.f;

	m.c[1].x = 0.f;
	m.c[1].y = a;
	m.c[1].z = 0.f;
	m.c[1].w = 0.f;

	m.c[2].x = 0.f;
	m.c[2].y = 0.f;
	m.c[2].z = -((f + n) / (f - n));
	m.c[2].w = -1.f;

	m.c[3].x = 0.f;
	m.c[3].y = 0.f;
	m.c[3].z = -((2.f * f * n) / (f - n));
	m.c[3].w = 0.f;

	return m;
}
static inline Mat4 mat4_look_at(const Vec3 eye, const Vec3 center, const Vec3 up) {
	Mat4 m;
	Vec3 f, s, t;

	f = vec3_sub(center, eye);
	f = vec3_norm(f);

	s = vec3_mul_cross(f, up);
	s = vec3_norm(s);

	t = vec3_mul_cross(s, f);

	m.c[0].x = s.x;
	m.c[0].y = t.x;
	m.c[0].z = -f.x;
	m.c[0].w = 0.f;

	m.c[1].x = s.y;
	m.c[1].y = t.y;
	m.c[1].z = -f.y;
	m.c[1].w = 0.f;

	m.c[2].x = s.z;
	m.c[2].y = t.z;
	m.c[2].z = -f.z;
	m.c[2].w = 0.f;

	m.c[3].x = 0.f;
	m.c[3].y = 0.f;
	m.c[3].z = 0.f;
	m.c[3].w = 1.f;

	mat4_translate_in_place(&m, -eye.x, -eye.y, -eye.z);

	return m;
}

#endif
