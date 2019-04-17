#version 330 core

out vec4 color;

in vec3 o_pos;
in vec3 o_normals;

uniform vec3 eye_pos;
uniform samplerCube skybox;

void main(){
	vec3 incidence = normalize(o_pos - eye_pos);
	vec3 reflection = reflect(incidence, normalize(o_normals));
		
	color = vec4(texture(skybox, reflection).rgb, 1.0);
	
}