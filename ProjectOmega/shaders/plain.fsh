#version 330 core

out vec4 color;

in vec3 o_pos;
in vec2 o_tex;
//in vec4 o_color;

uniform sampler2D poolTexture;

void main(){
	color = texture(poolTexture, o_tex);
	//color = o_color;
}