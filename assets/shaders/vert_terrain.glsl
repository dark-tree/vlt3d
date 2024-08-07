#version 450

layout(push_constant) uniform UniformBufferObject {
    mat4 mvp;
} uObject;

layout(location = 0) in vec3 iPosition;
layout(location = 1) in vec2 iTexture;
layout(location = 2) in vec3 iColor;
layout(location = 3) in uint iNorm;

layout(location = 0) out vec3 vColor;
layout(location = 1) out vec2 vTexture;
layout(location = 2) out vec3 vNorm;

void main() {

    vec3 normals[6] = {
        {-1, 0, 0},
        {+1, 0, 0},
        {0, -1, 0},
        {0, +1, 0},
        {0, 0, -1},
        {0, 0, +1},
    };

    gl_Position = uObject.mvp * vec4(iPosition, 1.0);
    vColor = iColor;
    vTexture = iTexture;
    vNorm = normals[iNorm];
}