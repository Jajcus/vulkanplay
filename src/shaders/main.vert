#version 420 core

layout(std140, binding = 0) uniform buf {
        mat4 mvp_matrix;
        mat4 mv_matrix;
        mat4 normal_matrix;
	vec4 light_pos;
} ubuf;

layout(location = 0) in vec4 in_position;
layout(location = 1) in vec4 in_normal;
layout(location = 2) in vec4 in_color;

layout(location = 0) out vec4 v_color;

out gl_PerVertex {
  vec4 gl_Position;
};

const vec4 light_ambient = vec4( 0.05f, 0.05f, 0.05f, 1.0f );
const vec4 light_diffuse = vec4( 1.0f, 1.0f, 1.0f, 1.0f );

void main() {

	gl_Position = ubuf.mvp_matrix * in_position;

	vec4 ambient = light_ambient * in_color;
	ambient = clamp(ambient, 0.0, 1.0); 
	
	vec3 V = vec3(ubuf.mv_matrix * in_position);
	vec3 N = vec3(normalize(ubuf.normal_matrix * in_normal));
	vec3 L = normalize(vec3(ubuf.mv_matrix * ubuf.light_pos) - V);

	vec4 diffuse = light_diffuse * in_color * max(dot(N, L), 0.0); 
	
	diffuse = clamp(diffuse, 0.0, 1.0); 

	v_color = ambient + diffuse;
}
