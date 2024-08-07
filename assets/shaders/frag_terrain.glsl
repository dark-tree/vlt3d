#version 450

struct LightSource {
    vec3 pos;
    uint color;
};

layout(push_constant) uniform UniformBufferObject {
    layout(offset = 64) LightSource sun;
} uObject;

layout(binding = 0) uniform sampler2D uSampler;

layout(location = 0) in vec3 vColor;
layout(location = 1) in vec2 vTexture;
layout(location = 2) in vec3 vNorm;
layout(location = 3) in vec3 vPosition;

layout(location = 0) out vec4 fColor;

void main() {

    // is this needed?
    vec3 normal = normalize(vNorm);

    // ambient lighting
    float ambient = 0;
    ambient += (vNorm.x < -0.1) ? 0.02 : 0;
    ambient += (vNorm.x > +0.1) ? 0.05 : 0;
    ambient += (vNorm.y < -0.1) ? 0.02 : 0;
    ambient += (vNorm.y > +0.1) ? 0.07 : 0;
    ambient += (vNorm.z < -0.1) ? 0.03 : 0;
    ambient += (vNorm.z > +0.1) ? 0.04 : 0;

    // diffuse lighting
    vec4 diffuse = vec4(unpackUnorm4x8(uObject.sun.color).rgb * max(dot(normal, uObject.sun.pos), 0.0), 1.0);

    fColor = texture(uSampler, vTexture).rgba * (diffuse + vec4(ambient));
}