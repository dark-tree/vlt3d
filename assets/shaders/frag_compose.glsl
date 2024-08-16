#version 450

layout(input_attachment_index = 0, binding = 0) uniform subpassInput uNormalInput;
layout(input_attachment_index = 1, binding = 2) uniform subpassInput uAlbedoInput;

layout(binding = 1) uniform sampler2D uPositionSampler;
layout(binding = 3) uniform sampler2D uAmbienceSampler;

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

float getAmbientOcclusion() {

    vec2 texel = 1.0 / vec2(textureSize(uAmbienceSampler, 0));
    float result = 0.0;

    for (int x = -2; x < 2; x ++)  {
        for (int y = -2; y < 2; y ++) {
            vec2 offset = vec2(float(x), float(y)) * texel;
            result += texture(uAmbienceSampler, vTexture + offset).r;
        }
    }

    return result / (4.0 * 4.0);

}

void main() {

    // Read G-Buffer values from previous subpass
    vec4 normal = uSceneObject.light * vec4(subpassLoad(uNormalInput).xyz, 0.0f);
    vec3 albedo = subpassLoad(uAlbedoInput).rgb;
    float ambience = getAmbientOcclusion();
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

    fColor = vec4(albedo * pow(ambience, 6) * (diffuse + ambient), 1.0);
}