#version 450

layout(push_constant) uniform SceneUniform {
    mat4 mvp;
    mat4 view;
} uSceneObject;

layout(location = 0) in vec3 iPosition;
layout(location = 1) in uint iSprite;
layout(location = 2) in uint iTexture;
layout(location = 3) in vec3 iColor;
layout(location = 4) in uint iNorm;

layout(location = 0) out vec3 vColor;
layout(location = 1) out vec3 vTexture;
layout(location = 2) out vec3 vNormal;
layout(location = 3) out vec3 vPosition;

void main() {

    vec3 normals[6] = {
        {-1, 0, 0},
        {+1, 0, 0},
        {0, -1, 0},
        {0, +1, 0},
        {0, 0, -1},
        {0, 0, +1},
    };

    // TODO any better ideas? Can we do the SSAO in some other coordinate space?
    mat3 normal_matrix = transpose(inverse(mat3(uSceneObject.view)));

    gl_Position = uSceneObject.mvp * vec4(iPosition, 1.0);
    vColor = iColor;
    vTexture = vec3(unpackHalf2x16(iTexture), iSprite);

    // view space
    vNormal = normal_matrix * normals[iNorm];
    vPosition = (uSceneObject.view *  vec4(iPosition, 1.0)).xyz;
}