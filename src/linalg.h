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
	Vec4 a;
	Vec4 b;
	Vec4 c;
	Vec4 d;
} Mat4;

typedef union vec34 {
	Vec4 v4;
	Vec3 v3;
} Vec34;

static inline int linalg_sanity_ok(void) {

	return ( sizeof(Vec3) == 3 * sizeof(float)
		&& sizeof(Vec4) == 4 * sizeof(float)
		&& sizeof(Mat4) == 16 * sizeof(float)
		&& sizeof(Vec34) == sizeof(Vec4) );
}

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

static const Mat4 MAT4_IDENTITY = {
	{ 1, 0, 0, 0 },
	{ 0, 1, 0, 0 },
	{ 0, 0, 1, 0 },
	{ 0, 0, 0, 1 }};

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
	m.a = vec4_add(a.a, b.a);
	m.b = vec4_add(a.b, b.b);
	m.c = vec4_add(a.c, b.c);
	m.d = vec4_add(a.d, b.d);
	return m;
}
static inline Mat4 mat4_sub(const Mat4 a, const Mat4 b) {
	Mat4 m;
	m.a = vec4_sub(a.a, b.a);
	m.b = vec4_sub(a.b, b.b);
	m.c = vec4_sub(a.c, b.c);
	m.d = vec4_sub(a.d, b.d);
	return m;
}
static inline Mat4 mat4_scale(const Mat4 a, float k) {
	Mat4 m;
	m.a = vec4_scale(a.a, k);
	m.b = vec4_scale(a.b, k);
	m.c = vec4_scale(a.c, k);
	m.d = vec4_scale(a.d, k);
	return m;
}
static inline Mat4 mat4_scale_aniso(const Mat4 a, float x, float y, float z) {
	Mat4 m;
	m.a = vec4_scale(a.a, x);
	m.b = vec4_scale(a.b, y);
	m.c = vec4_scale(a.c, z);
	m.d = a.d;
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

	t.d.x = x;
	t.d.y = y;
	t.d.z = z;
	return t;
}

static inline void mat4_translate_in_place(Mat4 * m, float x, float y, float z) {
	Vec4 t = {x, y, z, 0};

	Vec4 rx = {m->a.x, m->b.x, m->c.x, m->d.x};
	m->d.x += vec4_mul_inner(rx, t);
	Vec4 ry = {m->a.y, m->b.y, m->c.y, m->d.y};
	m->d.y += vec4_mul_inner(ry, t);
	Vec4 rz = {m->a.z, m->b.z, m->c.z, m->d.z};
	m->d.z += vec4_mul_inner(rz, t);
	Vec4 rw = {m->a.w, m->b.w, m->c.w, m->d.w};
	m->d.w += vec4_mul_inner(rw, t);
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

		Mat4 S = {{0, u.z, -u.y, 0},
			    {-u.z, 0, u.x, 0},
			    {u.y, -u.x, 0, 0},
			    {0, 0, 0, 0}};

		S = mat4_scale(S, s);

		Mat4 C = mat4_sub(MAT4_IDENTITY, T);

		C = mat4_scale(C, c);

		T = mat4_add(T, C);
		T = mat4_add(T, S);

		T.d.w = 1.0;
		return mat4_mul(M, T);
	}
	else {
		return M;
	}
}
static inline Mat4 mat4_rotate_X(const Mat4 M, float angle) {
    float s = sinf(angle);
    float c = cosf(angle);
    Mat4 R = {{1.f, 0.f, 0.f, 0.f},
                {0.f, c, s, 0.f},
                {0.f, -s, c, 0.f},
                {0.f, 0.f, 0.f, 1.f}};
    return mat4_mul(M, R);
}
static inline Mat4 mat4_rotate_Y(const Mat4 M, float angle) {
    float s = sinf(angle);
    float c = cosf(angle);
    Mat4 R = {{c, 0.f, s, 0.f},
                {0.f, 1.f, 0.f, 0.f},
                {-s, 0.f, c, 0.f},
                {0.f, 0.f, 0.f, 1.f}};
    return mat4_mul(M, R);
}
static inline Mat4 mat4_rotate_Z(const Mat4 M, float angle) {
    float s = sinf(angle);
    float c = cosf(angle);
    Mat4 R = {{c, s, 0.f, 0.f},
                {-s, c, 0.f, 0.f},
                {0.f, 0.f, 1.f, 0.f},
                {0.f, 0.f, 0.f, 1.f}};
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
		(Vec3 *)&r.a,
		(Vec3 *)&r.b,
		(Vec3 *)&r.c,
		(Vec3 *)&r.d,
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

	m.a.x = 2.f * n / (r - l);
	m.a.y = m.a.z = m.a.w = 0.f;

	m.b.y = 2.f * n / (t - b);
	m.b.x = m.b.z = m.b.w = 0.f;

	m.c.x = (r + l) / (r - l);
	m.c.y = (t + b) / (t - b);
	m.c.z = -(f + n) / (f - n);
	m.c.w = -1.f;

	m.d.z = -2.f * (f * n) / (f - n);
	m.d.x = m.d.y = m.d.w = 0.f;

	return m;
}
static inline Mat4 mat4_ortho(float l, float r, float b, float t, float n, float f) {
	Mat4 m;
	m.a.x = 2.f / (r - l);
	m.a.y = m.a.z = m.a.w = 0.f;

	m.b.y = 2.f / (t - b);
	m.b.x = m.b.z = m.b.w = 0.f;

	m.c.z = -2.f / (f - n);
	m.c.x = m.c.y = m.c.w = 0.f;

	m.d.x = -(r + l) / (r - l);
	m.d.y = -(t + b) / (t - b);
	m.d.z = -(f + n) / (f - n);
	m.d.w = 1.f;
	return m;
}
static inline Mat4 mat4_perspective(float y_fov, float aspect, float n, float f) {
	Mat4 m;
	float const a = (float)(1.f / tan(y_fov / 2.f));

	m.a.x = a / aspect;
	m.a.y = 0.f;
	m.a.z = 0.f;
	m.a.w = 0.f;

	m.b.x = 0.f;
	m.b.y = a;
	m.b.z = 0.f;
	m.b.w = 0.f;

	m.c.x = 0.f;
	m.c.y = 0.f;
	m.c.z = -((f + n) / (f - n));
	m.c.w = -1.f;

	m.d.x = 0.f;
	m.d.y = 0.f;
	m.d.z = -((2.f * f * n) / (f - n));
	m.d.w = 0.f;

	return m;
}
static inline Mat4 mat4_view(const Vec3 eye_pos, const Vec3 eye_dir, const Vec3 up) {
	Mat4 m;
	Vec3 f, s, t;

	f = vec3_norm(eye_dir);

	s = vec3_mul_cross(f, up);
	s = vec3_norm(s);

	t = vec3_mul_cross(s, f);

	m.a.x = s.x;
	m.a.y = t.x;
	m.a.z = -f.x;
	m.a.w = 0.f;

	m.b.x = s.y;
	m.b.y = t.y;
	m.b.z = -f.y;
	m.b.w = 0.f;

	m.c.x = s.z;
	m.c.y = t.z;
	m.c.z = -f.z;
	m.c.w = 0.f;

	m.d.x = 0.f;
	m.d.y = 0.f;
	m.d.z = 0.f;
	m.d.w = 1.f;

	mat4_translate_in_place(&m, -eye_pos.x, -eye_pos.y, -eye_pos.z);

	return m;
}

#endif
