#version 330 core

in vec3 o_pos;
in vec2 o_texcoords;

uniform sampler2D frametex;

out vec4 frame_color;

const float offset = 1.0 / 300.0;

void main(){
	
	//kernel stuff with memery involved
	vec2 offsets[9] = vec2[](
		vec2(-offset,  offset), // top-left
        vec2( 0.0f,    offset), // top-center
        vec2( offset,  offset), // top-right
        vec2(-offset,  0.0f),   // center-left
        vec2( 0.0f,    0.0f),   // center-center
        vec2( offset,  0.0f),   // center-right
        vec2(-offset, -offset), // bottom-left
        vec2( 0.0f,   -offset), // bottom-center
        vec2( offset, -offset)
	);

	float kernel[9] = float[](
        1.f/16, 2.f/16, 1.f/16,
		2.f/16, 4.f/16, 2.f/16,
		1.f/16, 2.f/16, 1.f/16
    );
	
	vec3 sampleTex[9];
	for(int i = 0; i < 9; i++)
    {
        sampleTex[i] = vec3(texture(frametex, o_texcoords + offsets[i]));
    }

	vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++)
        col += sampleTex[i] * kernel[i];
	

	frame_color = vec4(col, 1.0);

}