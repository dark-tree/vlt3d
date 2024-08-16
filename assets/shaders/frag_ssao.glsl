#version 450

layout(binding = 0) uniform AmbientOcclusionUniform {
    vec4 samples[64];
} uAmbientObject;

layout(push_constant) uniform SceneUniform {
    mat4 projection;
} uSceneObject;

layout(binding = 1) uniform sampler2D uNoiseSampler;
layout(binding = 2) uniform sampler2D uNormalSampler;
layout(binding = 3) uniform sampler2D uPositionSampler;
layout(binding = 4) uniform sampler2D uAlbedoSampler;

layout(location = 0) in vec2 vTexture;
layout(location = 0) out float fAmbience;

const vec2 noise_scale = vec2(1000.0/4.0, 700.0/4.0);
const float bias = 0.025;
const int kernel_size = 64;
const float radius = 0.9;

void main() {

    vec3 position = texture(uPositionSampler, vTexture).xyz;
    vec3 normal = texture(uNormalSampler, vTexture).rgb;
    vec3 random = texture(uNoiseSampler, vTexture * noise_scale).xyz;

    // Create a TBN matrix that transforms any vector from tangent-space to view-space.
    // Using a process called the "Gramm-Schmidt process" we create an orthogonal basis, each time slightly tilted
    // based on the value of 'random'.
    vec3 tangent = normalize(random - normal * dot(random, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;

    for (int i = 0; i < kernel_size; i ++) {

        // get sample position, from tangent to view-space
        vec3 point = TBN * uAmbientObject.samples[i].xyz;
        vec3 pos = position + point * radius;

        // project sample position from view to clip-space
        vec4 offset = vec4(pos, 1.0);
        offset = uSceneObject.projection * offset;
        offset.xyz = (offset.xyz / offset.w) * 0.5 + 0.5;

        float depth = texture(uPositionSampler, offset.xy).z;
        float limit = smoothstep(0.0, 1.0, radius / (8 * abs(position.z - depth)));
        occlusion += (depth >= pos.z + bias ? 1.0 : 0.0) * limit;
    }

    fAmbience = 1.0f - (occlusion / kernel_size);

}