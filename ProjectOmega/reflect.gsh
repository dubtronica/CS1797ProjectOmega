#version 330 compatibility
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec3 g_pos[3];
in vec4 g_clipSpace[3];

out vec3 o_pos;
out vec3 o_normals;
out vec4 clipSpace;

vec3 getNormal()
{
	vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
	vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
	return normalize(cross(a, b));
}

void main()
{
	for (int i = 0; i < 3; i++)
	{
		gl_Position = gl_in[i].gl_Position;
		o_pos = g_pos[i];
		o_normals = getNormal();
		clipSpace = g_clipSpace[i];
		EmitVertex();
	}
	EndPrimitive();
}