#version 450

layout(binding = 0) uniform AmbientOcclusionUniform {
    vec4 samples[64];
} uAmbientObject;

layout(push_constant) uniform SceneUniform {
    mat4 projection;
    mat4 view;
} uSceneObject;

layout(binding = 1) uniform sampler2D uNoiseSampler;
layout(binding = 2) uniform sampler2D uNormalSampler;
layout(binding = 3) uniform sampler2D uPositionSampler;
layout(binding = 4) uniform sampler2D uAlbedoSampler;

layout(location = 0) in vec2 vTexture;
layout(location = 0) out vec3 fAmbience;

const vec2 noiseScale = vec2(1000.0/4.0, 700.0/4.0);
const float bias = 0.025;
const int kernel_size = 64;
const float radius = 0.8;

void main() {

    vec3 position = texture(uPositionSampler, vTexture).xyz;
    vec3 normal = texture(uNormalSampler, vTexture).rgb;
    vec3 random = texture(uNoiseSampler, vTexture * noiseScale).xyz;
    vec3 albedo = mix(texture(uAlbedoSampler, vTexture).rgb, vec3(1), 0.5f);

    vec3 tangent = normalize(random - normal * dot(random, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;

    for (int i = 0; i < kernel_size; i ++) {

        // get sample position
        vec3 samplePos = TBN * uAmbientObject.samples[i].xyz; // from tangent to view-space
        samplePos = position + samplePos * radius;

        vec4 offset = vec4(samplePos, 1.0);
        offset      = uSceneObject.projection * offset;    // from view to clip-space
        offset.xyz /= offset.w;               // perspective divide
        offset.xyz  = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0

        float sampleDepth = texture(uPositionSampler, vec2(offset.x, offset.y)).z;

//        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0);

        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(position.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
    }

    fAmbience = vec3(albedo - (occlusion / kernel_size));

}