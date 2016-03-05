#version 420 core

struct material_s {
	vec4 color;
};

layout(constant_id = 1) const int materials_len = 1;

layout(std140, binding = 0) uniform buf {
	vec4 light_pos;
	material_s materials[materials_len];
} ubuf;

/* veertex data */
layout(location = 0) in vec4 in_position;
layout(location = 1) in vec4 in_normal;
layout(location = 2) in uint in_material;

/* instance data */
layout(location = 4) in mat4 mv_matrix;
layout(location = 8) in mat4 mvp_matrix;
layout(location = 12) in mat4 normal_matrix;

layout(location = 0) out vec4 v_color;

out gl_PerVertex {
  vec4 gl_Position;
};

const vec4 light_ambient = vec4( 0.05f, 0.05f, 0.05f, 1.0f );
const vec4 light_diffuse = vec4( 1.0f, 1.0f, 1.0f, 1.0f );

void main() {

	gl_Position = mvp_matrix * in_position;

	material_s material = ubuf.materials[in_material];

	vec4 ambient = light_ambient * material.color;
	ambient = clamp(ambient, 0.0, 1.0); 
	
	vec3 V = vec3(mv_matrix * in_position);
	vec3 N = vec3(normalize(normal_matrix * in_normal));
	vec3 L = normalize(vec3(mv_matrix * ubuf.light_pos) - V);

	vec4 diffuse = light_diffuse * material.color * max(dot(N, L), 0.0); 
	
	diffuse = clamp(diffuse, 0.0, 1.0); 

	v_color = ambient + diffuse;
}
