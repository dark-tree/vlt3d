#version 450

layout(push_constant) uniform UniformBufferObject {
    mat4 mvp;
} uObject;

layout(location = 0) in vec3 iPosition;
layout(location = 1) in vec2 iTexture;
layout(location = 2) in uint iColor;

layout(location = 0) out vec4 vColor;
layout(location = 1) out vec2 vTexture;

void main() {
    gl_Position = uObject.mvp * vec4(iPosition, 1.0);
    vColor = unpackUnorm4x8(iColor).rgba;
    vTexture = iTexture;
}