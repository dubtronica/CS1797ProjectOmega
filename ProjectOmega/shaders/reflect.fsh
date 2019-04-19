#version 330 core

out vec4 color;

in vec3 o_pos;
in vec3 o_normals;

uniform vec3 eye_pos;
uniform samplerCube skybox;

void main(){
	vec3 incidence = normalize(o_pos - eye_pos);

	float value = 1.000 / 1.333;

	float fresnel = dot(incidence, vec3(0.0, 1.0, 0.0));
	vec3 reflection = reflect(incidence, normalize(o_normals));
	vec3 refraction = refract(incidence, normalize(o_normals), value);

	vec4 reflectcolor = vec4(texture(skybox, reflection).rgb, 1.0);
	vec4 refractcolor = vec4(texture(skybox, refraction).rgb, 1.0);
		
	color = mix(reflectcolor, refractcolor, fresnel);
	
}