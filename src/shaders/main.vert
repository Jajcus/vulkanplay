#version 420 core

const uint V_FLAG_FLAT = 1;

struct material_s {
	vec4 ambient_color;
	vec4 diffuse_color;
	vec4 specular_color;
	float shininess, pad1, pad2, pad3;
};

struct light_s {
	vec4 position;
	vec4 diffuse;
	vec4 specular;
};

layout(constant_id = 1) const int materials_len = 3;
layout(constant_id = 2) const int lights_len = 1;

layout(std140, binding = 0) uniform buf {
	mat4 v_matrix;
	vec4 ambient_light;
	material_s materials[materials_len];
	light_s lights[lights_len];
} ubuf;

/* veertex data */
layout(location = 0) in vec4 in_position;
layout(location = 1) in vec4 in_normal;
layout(location = 2) in uint in_material;
layout(location = 3) in uint in_flags;

/* instance data */
layout(location = 4) in mat4 mv_matrix;
layout(location = 8) in mat4 mvp_matrix;
layout(location = 12) in mat4 normal_matrix;

layout(location = 0) out vec3 V;
layout(location = 1) out vec3 N;
layout(location = 2) out vec4 v_ambient_color;
layout(location = 3) out flat vec4 v_color_flat;
layout(location = 4) out flat uint v_flags;
layout(location = 5) out flat uint v_material;
layout(location = 6) out flat mat4 v_mv_matrix;

out gl_PerVertex {
  vec4 gl_Position;
};


void main() {

	gl_Position = mvp_matrix * in_position;

	V = vec3(mv_matrix * in_position);
	N = normalize(vec3(normal_matrix * in_normal));
	v_mv_matrix = mv_matrix;

	v_flags = in_flags;
	v_material = in_material;

	material_s material = ubuf.materials[in_material];

	vec4 ambient = ubuf.ambient_light * material.ambient_color;
	ambient = clamp(ambient, 0.0, 1.0);

	v_ambient_color = ambient;
	v_color_flat = v_ambient_color;

	if ((v_flags & V_FLAG_FLAT) == V_FLAG_FLAT) {
		for(int i = 0; i < lights_len; i++) {
			light_s light = ubuf.lights[i];

			vec3 L = normalize(vec3(ubuf.v_matrix * light.position) - V);

			float nl = dot(N, L);
			vec4 diffuse = light.diffuse * material.diffuse_color * max(nl, 0.0);
			diffuse = clamp(diffuse, 0.0, 1.0);
			v_color_flat += diffuse;

			if (nl >= 0.0) {
				vec3 R = normalize(-reflect(L, N));
				float re = dot(R, normalize(-V));
				vec4 specular = light.specular * material.specular_color * pow(max(re, 0.0), material.shininess);
				specular = clamp(specular, 0.0, 1.0);
				v_color_flat += specular;
			}
		}
	}
}
