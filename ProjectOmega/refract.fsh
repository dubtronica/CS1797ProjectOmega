#version 330 core

out vec4 color;

in vec3 o_pos;
in vec3 o_normals;

uniform vec3 eye_pos;
uniform samplerCube skybox;

void main(){
	float value = 1.00 / 1.52;
	vec3 incidence = normalize(o_pos - eye_pos);
	vec3 refraction = refract(incidence, normalize(o_normals), value);
		
	color = vec4(texture(skybox, refraction).rgb, 1.0);
	
}