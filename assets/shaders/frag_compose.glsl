#version 450

layout(input_attachment_index = 0, binding = 0) uniform subpassInput uAlbedoInput;
layout(input_attachment_index = 1, binding = 1) uniform subpassInput uNormalInput;
layout(input_attachment_index = 2, binding = 2) uniform subpassInput uPositionInput;

struct LightSource {
    vec3 pos;
    uint color;
};

layout(push_constant) uniform UniformBufferObject {
    layout(offset = 64) LightSource sun;
} uObject;

layout(location = 0) out vec4 pColor;

void main() {

    // Read G-Buffer values from previous subpass
    vec3 fPosition = subpassLoad(uPositionInput).rgb;
    vec3 fNormal = subpassLoad(uNormalInput).rgb;
    vec4 fAlbedo = subpassLoad(uAlbedoInput);

    // ambient lighting
    float ambient = 0;
    ambient += (fNormal.x < -0.1) ? 0.03 : 0;
    ambient += (fNormal.x > +0.1) ? 0.06 : 0;
    ambient += (fNormal.y < -0.1) ? 0.03 : 0;
    ambient += (fNormal.y > +0.1) ? 0.08 : 0;
    ambient += (fNormal.z < -0.1) ? 0.04 : 0;
    ambient += (fNormal.z > +0.1) ? 0.05 : 0;

    // diffuse lighting
    vec4 diffuse = vec4(unpackUnorm4x8(uObject.sun.color).rgb * max(dot(fNormal.xyz, uObject.sun.pos), 0.0), 1.0);

    pColor = fAlbedo * (diffuse + vec4(ambient * 2));
}