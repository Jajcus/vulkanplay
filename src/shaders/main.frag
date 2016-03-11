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

layout(location = 0) in vec3 V;
layout(location = 1) in vec3 N;
layout(location = 2) in vec4 v_ambient_color;
layout(location = 3) in flat vec4 v_color_flat;
layout(location = 4) in flat uint v_flags;
layout(location = 5) in flat uint v_material;
layout(location = 6) in flat mat4 v_mv_matrix;

layout(location = 0) out vec4 f_color;

void main() {

	if ((v_flags & V_FLAG_FLAT) == V_FLAG_FLAT) {
		f_color = v_color_flat;
	}
	else {
		material_s material = ubuf.materials[v_material];
		f_color = v_ambient_color;
		for(int i = 0; i < lights_len; i++) {
			light_s light = ubuf.lights[i];

			vec3 L = normalize(vec3(ubuf.v_matrix * light.position) - V);

			float nl = dot(N, L);
			vec4 diffuse = light.diffuse * material.diffuse_color * max(nl, 0.0);
			diffuse = clamp(diffuse, 0.0, 1.0);
			f_color += diffuse;

			if (nl >= 0.0) {
				vec3 R = normalize(-reflect(L, N));
				float re = dot(R, normalize(-V));
				vec4 specular = light.specular * material.specular_color * pow(max(re, 0.0), material.shininess);
				specular = clamp(specular, 0.0, 1.0);
				f_color += specular;
			}
		}
	}
}
