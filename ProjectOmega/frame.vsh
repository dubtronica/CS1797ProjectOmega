#version 330 core

layout(location = 0) in vec3 i_pos;
layout(location = 1) in vec2 i_texcoords;

out vec3 o_pos;
out vec2 o_texcoords;

void main() {
	gl_Position = vec4(i_pos, 1.0);
	o_pos = i_pos;
	o_texcoords = i_texcoords;
}