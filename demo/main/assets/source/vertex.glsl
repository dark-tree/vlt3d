#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 3) in vec3 color;
layout (location = 4) in mat4 mvp;

out vec3 col;

void main()
{    
    gl_Position = mvp * vec4(aPos, 1.0);
	col = color;
}
