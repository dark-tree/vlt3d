#version 460 core

out vec4 color;

in vec2 uv;

uniform sampler2D sampler;

void main() { 

	color = vec4(texture(sampler, uv).rgb, 1.0);

}

