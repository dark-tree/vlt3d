#version 450

layout(input_attachment_index = 0, binding = 5) uniform subpassInput uAmbienceInput;

layout(binding = 2) uniform sampler2D uNormalSampler;
layout(binding = 3) uniform sampler2D uPositionSampler;
layout(binding = 4) uniform sampler2D uAlbedoSampler;

//struct LightSource {
//    vec3 pos;
//    uint color;
//};
//
//layout(push_constant) uniform UniformBufferObject {
//    layout(offset = 64) LightSource sun;
//} uObject;

layout(location = 0) in vec2 vTexture;
layout(location = 0) out vec4 fColor;

void main() {

    // Read G-Buffer values from previous subpass
    vec3 normal = texture(uNormalSampler, vTexture).rgb;
    vec3 position = texture(uPositionSampler, vTexture).xyz;
    vec3 albedo = texture(uAlbedoSampler, vTexture).rgb;
    float ambience = subpassLoad(uAmbienceInput).r;

//    // ambient lighting
//    float ambient = 0;
//    ambient += (fNormal.x < -0.1) ? 0.03 : 0;
//    ambient += (fNormal.x > +0.1) ? 0.06 : 0;
//    ambient += (fNormal.y < -0.1) ? 0.03 : 0;
//    ambient += (fNormal.y > +0.1) ? 0.08 : 0;
//    ambient += (fNormal.z < -0.1) ? 0.04 : 0;
//    ambient += (fNormal.z > +0.1) ? 0.05 : 0;
//
//    // diffuse lighting
//    vec4 diffuse = vec4(unpackUnorm4x8(uObject.sun.color).rgb * max(dot(fNormal.xyz, uObject.sun.pos), 0.0), 1.0);

    fColor = vec4(albedo * pow(ambience, 5), 1.0);
}