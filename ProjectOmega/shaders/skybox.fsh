#version 330 core

in vec3 o_texcoords;

out vec4 color;

uniform samplerCube texCube;

void main(){
	color = texture(texCube, o_texcoords);
}