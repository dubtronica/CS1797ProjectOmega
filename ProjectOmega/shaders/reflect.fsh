#version 330 core

out vec4 color;

in vec3 o_pos;
in vec3 o_normals;
in vec4 clipSpace;

uniform vec3 eye_pos, lightpos, lightcolor;
uniform samplerCube skybox;
uniform sampler2D dudv;
uniform sampler2D pooltex;

void main(){
	vec2 ndc = (clipSpace.xy / clipSpace.w) / 2.0 + 0.5;

	vec3 incidence = normalize(o_pos - eye_pos);

	float value = 1.000 / 1.333;
	float transparent = 1.000 / 1.000;

	float fresnel = dot(incidence, o_normals);
	vec3 reflection = reflect(incidence, normalize(o_normals));
	vec3 refraction = refract(incidence, normalize(o_normals), value);

	vec3 transparency = refract(incidence, normalize(o_normals), transparent);

	if(fresnel < 0.15){
		fresnel = 0.15;
	} 

	vec3 reflectlight = reflect(normalize(lightpos - o_pos), normalize(o_normals));
	float specular = max( dot(reflectlight, incidence), 0.0);
	specular = pow(specular, 20.0);
	vec3 highlights = lightcolor * specular * 0.2;

	vec4 wallcolor = texture(pooltex, ndc);

	vec4 reflectcolor = vec4(texture(skybox, reflection).rgb, 1.0);
	vec4 refractcolor = mix(wallcolor, vec4(texture(skybox, refraction).rgb, 1.0), 0.45);
		
	color = mix(mix(reflectcolor, refractcolor, fresnel), vec4(0.0, 0.5, 0.5, 1.0), 0.6);

}