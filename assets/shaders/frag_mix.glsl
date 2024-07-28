#version 450

layout(binding = 1) uniform sampler2D uSampler;

layout(location = 0) in vec4 vColor;
layout(location = 1) in vec2 vTexture;

layout(location = 0) out vec4 fColor;

void main() {
    fColor = mix(texture(uSampler, vTexture).rgba, vColor, 0.5);

    // 1/250
    if (fColor.a <= 0.004) {
        discard;
    }
}