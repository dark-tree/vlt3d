#version 450

layout(push_constant) uniform ChunkUniform {
    vec3 offset;
} uChunkObject;

layout(binding = 0) uniform SceneUniform {
    mat4 mvp;
} uSceneObject;

layout(location = 0) in vec3 iPosition;

void main() {
    gl_Position = uSceneObject.mvp * vec4(uChunkObject.offset + iPosition, 1.0);
}
