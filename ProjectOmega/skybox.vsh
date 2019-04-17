#version 330 core

layout(location = 0) in vec3 v_pos;

out vec3 o_texcoords;

uniform mat4 projection;
uniform mat4 view;

void main() {
	o_texcoords = v_pos;
	vec4 pos = projection * view * vec4(v_pos, 1.0);
	gl_Position = pos.xyww;
}