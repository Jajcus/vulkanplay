#version 420 core

layout(location = 0) in vec4 v_color;

layout(location = 0) out vec4 f_color;

void main() {
	f_color = v_color;
	//f_color = vec4(0.0, 1.0, 0.0, 1.0); // green
}