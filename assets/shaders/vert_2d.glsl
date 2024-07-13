#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} uObject;

layout(location = 0) in vec2 iPosition;
layout(location = 1) in vec2 iTexture;
layout(location = 2) in uint iColor;

layout(location = 0) out vec4 vColor;
layout(location = 1) out vec2 vTexture;

void main() {
    gl_Position = vec4(iPosition, 0.0, 1.0);
    vColor = unpackUnorm4x8(iColor).rgba;
    vTexture = iTexture;
}