#version 450

layout(binding = 0) uniform sampler2D uAtlasSampler;

struct LightSource {
    vec3 pos;
    uint color;
};

layout(push_constant) uniform UniformBufferObject {
    layout(offset = 64) LightSource sun;
} uObject;

layout(location = 0) in vec3 vColor;
layout(location = 1) in vec2 vTexture;
layout(location = 2) in vec3 vNormal;
layout(location = 3) in vec3 vPosition;

layout(location = 0) out vec4 fAlbedo;
layout(location = 1) out vec4 fNormal;
layout(location = 2) out vec4 fPosition;

void main() {

    // do we need to normalize vNormal here?
    fAlbedo = texture(uAtlasSampler, vTexture).rgba * vec4(vColor, 1.0f);
    fNormal = vec4(normalize(vNormal), 0.0f);
    fPosition = vec4(vPosition, 0);

}