#version 330 core
layout(location = 0) out vec4 out_color;

uniform float alpha;

void main(void) 
{
	out_color = vec4( 1,0.5,0.5, alpha);	
}

