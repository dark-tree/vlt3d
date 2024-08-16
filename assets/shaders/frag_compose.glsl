#version 450

layout(input_attachment_index = 0, binding = 5) uniform subpassInput uAmbienceInput;

layout(binding = 2) uniform sampler2D uNormalSampler;
layout(binding = 3) uniform sampler2D uPositionSampler;
layout(binding = 4) uniform sampler2D uAlbedoSampler;

struct LightSource {
    vec3 pos;
    uint color;
};

layout(push_constant) uniform SceneUniform {
    mat4 light; // inverse view matrix
    LightSource sun;
} uSceneObject;

layout(location = 0) in vec2 vTexture;
layout(location = 0) out vec4 fColor;

void main() {

    // Read G-Buffer values from previous subpass
    vec4 normal = uSceneObject.light * vec4(texture(uNormalSampler, vTexture).rgb, 0.0f);
//    vec3 position = texture(uPositionSampler, vTexture).xyz;
    vec3 albedo = texture(uAlbedoSampler, vTexture).rgb;
    float ambience = subpassLoad(uAmbienceInput).r;
    vec3 sun = unpackUnorm4x8(uSceneObject.sun.color).rgb;

    // ambient lighting
    vec3 ambient = 0.01f + sun * 0.08f;
    ambient += (normal.x < -0.1) ? 0.05 : 0;
    ambient += (normal.x > +0.1) ? 0.08 : 0;
    ambient += (normal.y < -0.1) ? 0.05 : 0;
    ambient += (normal.y > +0.1) ? 0.10 : 0;
    ambient += (normal.z < -0.1) ? 0.06 : 0;
    ambient += (normal.z > +0.1) ? 0.07 : 0;

    // diffuse lighting
    vec3 diffuse = sun * max(dot(normal.xyz, uSceneObject.sun.pos), 0.0);

    fColor = vec4(albedo * pow(ambience, 5) * (diffuse + ambient), 1.0);
}