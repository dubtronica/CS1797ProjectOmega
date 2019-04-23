#version 330 core

layout(location = 0) in vec3 v_pos; 
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_tex;

out vec2 o_texcoords;
// out vec4 o_color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec4 clipping_plane;

void main() {
	gl_ClipDistance[0] = dot(model * vec4(v_pos, 1.0), clipping_plane);

	o_texcoords = v_tex;
	// o_color = vec4(1.0);
	gl_Position = projection * view * model * vec4(v_pos, 1.0);
}
