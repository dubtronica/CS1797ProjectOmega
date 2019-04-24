#version 330 core

out vec4 color;

in vec3 o_pos;
in vec2 o_texcoords;
//in vec4 o_color;

uniform sampler2D poolTexture;
uniform sampler2D caustics[5];
uniform int time;

void main(){

	vec4 loop = texture(poolTexture, o_texcoords);

	vec4 caus = texture(caustics[time], o_texcoords);;

	color = mix(mix(loop, caus, 0.3), vec4(0, 1, 1, 0.4), 0.6);
	
}