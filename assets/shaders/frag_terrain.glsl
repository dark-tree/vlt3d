#version 450

layout(binding = 0) uniform sampler2D uSampler;

layout(location = 0) in vec3 vColor;
layout(location = 1) in vec2 vTexture;
layout(location = 2) in vec3 vNorm;

layout(location = 0) out vec4 fColor;

void main() {

    float light = 0;

    light += (vNorm.x < -0.1) ? 0.05 : 0;
    light += (vNorm.x > +0.1) ? 0.2 : 0;
    light += (vNorm.y < -0.1) ? 0.02 : 0;
    light += (vNorm.y > +0.1) ? 0.3 : 0;
    light += (vNorm.z < -0.1) ? 0.08 : 0;
    light += (vNorm.z > +0.1) ? 0.35 : 0;

    fColor = texture(uSampler, vTexture).rgba * light;
}