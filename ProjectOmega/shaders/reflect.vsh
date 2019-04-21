#version 330 core

layout(location = 0) in vec3 v_pos; 
layout(location = 1) in vec3 v_normals;
layout(location = 2) in vec2 v_texcoords;

out vec3 o_pos;
out vec3 o_normals;
out vec2 o_texcoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
	o_normals = mat3(transpose(inverse(model))) * v_normals;
	o_pos = vec3(model * vec4(v_pos, 1.0));
	gl_Position = projection * view * model * vec4(v_pos, 1.0);
	o_texcoords = v_texcoords;
}
