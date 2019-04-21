#version 330 core

out vec4 color;

in vec3 o_pos;
in vec3 o_normals;
in vec2 o_texcoords;

uniform vec3 eye_pos;
uniform samplerCube skybox;
uniform sampler2D dudv;
uniform sampler2D pooltex;

void main(){
	vec3 incidence = normalize(o_pos - eye_pos);

	float value = 1.000 / 1.333;
	float transparent = 1.000 / 1.000;

	float fresnel = dot(incidence, vec3(0.0, 1.0, 0.0));
	vec3 reflection = reflect(incidence, normalize(o_normals));
	vec3 refraction = refract(incidence, normalize(o_normals), value);

	vec3 transparency = refract(incidence, normalize(o_normals), transparent);

	if(fresnel < 0.15){
		fresnel = 0.15;
	} 

	vec4 wallcolor = texture(pooltex, o_texcoords);

	vec4 reflectcolor = vec4(texture(skybox, reflection).rgb, 1.0);
	vec4 refractcolor = mix(wallcolor, vec4(texture(skybox, refraction).rgb, 1.0), 0.5);
		
	color = mix(mix(reflectcolor, refractcolor, fresnel), vec4(0.0, 0.5, 0.5, 1.0), 0.6);
	
}