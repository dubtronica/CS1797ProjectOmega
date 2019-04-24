#version 330 core

layout(location = 0) in vec3 v_pos;
layout(location = 1) in vec3 v_normals;

out vec3 g_pos;
out vec4 g_clipSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float time;
uniform int style;

void main() {
	vec3 temp_pos = v_pos;
	if (style % 3 == 0)
	{
		temp_pos.y += sin(temp_pos.z * 100 + time * 10) / 50;
	}
	else if (style % 3 == 1)
	{
		temp_pos.y += sin(temp_pos.x * 100 + time * 10) / 50;
	}
	else
	{
		if (pow(pow(temp_pos.x, 2) + pow(temp_pos.z, 2), 0.5) - (sin(time) / 4) < 0.03)
			temp_pos.y -= 0.01;
		else if (pow(pow(temp_pos.x, 2) + pow(temp_pos.z, 2), 0.5) - (sin(time) / 4) < 0.02)
			temp_pos.y += 0.02;
		else if (pow(pow(temp_pos.x, 2) + pow(temp_pos.z, 2), 0.5) - (sin(time) / 4) < 0.01)
			temp_pos.y -= 0.01;
	}

	g_pos = vec3(model * vec4(temp_pos, 1.0));
	gl_Position = projection * view * model * vec4(v_pos, 1.0);
	g_clipSpace = gl_Position;
}
