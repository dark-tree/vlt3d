#version 460 core

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_normal;
layout (location = 2) in vec2 vertex_uv;

out vec3 position;
out vec3 normal;
out vec2 uv;

uniform mat4 view_projection_mat;
uniform mat4 model_mat;
uniform mat3 normal_mat;

void main() {

	/* 
	 * I'm not 100% sure if normalizing normal
	 * values in vertex shader is the correct way to do this
	 * the original code examples i read done this in 
	 * fragment shaders, but i didn't notice any changes
	 * after moving it here
	 */
	
	position = vec3(model_mat * vec4(vertex_position, 1.0));
	normal = normalize(normal_mat * vertex_normal);  
	uv = vertex_uv;	

	gl_Position = view_projection_mat * vec4(position, 1.0f);

}
