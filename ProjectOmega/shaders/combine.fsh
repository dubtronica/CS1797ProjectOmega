#version 330 core

in vec3 o_pos;
in vec2 o_texcoords;

uniform sampler2D pristineTex;
uniform sampler2D blurTex;
uniform sampler2D depthTex;

uniform float focus;
uniform int selection;

out vec4 comb_color;

void main(){
	
	float clarity = clamp(3 * abs(texture(depthTex, o_texcoords).r - focus), 0.0, 1.0);

	vec4 gscale1 = texture(pristineTex, o_texcoords);
	vec4 gscale2 = mix(texture(pristineTex, o_texcoords), texture(blurTex, o_texcoords), clarity);

	float gray = 0.2177 * gscale1.r + 0.5978 * gscale1.g + 0.4322 * gscale1.b;
	float gray2 = 0.2177 * gscale2.r + 0.5978 * gscale2.g + 0.4322 * gscale2.b;

	if(selection == 1){
		comb_color = gscale1;
	} else if(selection == 2){
		comb_color = vec4(vec3(gray), 1.0);
	} else if(selection == 3){
		comb_color = gscale2;
	} else if(selection == 4){
		comb_color = vec4(vec3(gray2), 1.0);
	}
}