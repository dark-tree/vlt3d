#version 450

layout(binding = 0) uniform SceneUniform {
    mat4 mvp;
    mat4 view;
    mat4 normal;
} uSceneObject;

layout(location = 0) in vec3 iPosition;
layout(location = 1) in vec2 iTexture;
layout(location = 2) in vec4 iColor;

layout(location = 0) out vec4 vColor;
layout(location = 1) out vec2 vTexture;

void main() {
    gl_Position = uSceneObject.mvp * vec4(iPosition, 1.0);
    vColor = iColor;
    vTexture = iTexture;
}