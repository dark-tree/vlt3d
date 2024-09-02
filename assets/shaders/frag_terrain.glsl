#version 450

layout(binding = 1) uniform sampler2DArray uArraySampler;

layout(location = 0) in vec3 vColor;
layout(location = 1) in vec3 vTexture;
layout(location = 2) in vec3 vNormal;
layout(location = 3) in vec3 vPosition;

layout(location = 0) out vec4 fAlbedo;
layout(location = 1) out vec4 fNormal;
layout(location = 2) out vec4 fPosition;

void main() {

    // do we need to normalize vNormal here?
    fAlbedo = texture(uArraySampler, vTexture).rgba ; //* vec4(vColor, 1.0f);
    fNormal = vec4(normalize(vNormal), 0.0f);
    fPosition = vec4(vPosition, 0);

//    // wireframe view
//    vec3 d = fwidth(vColor);
//    vec3 a3 = smoothstep(vec3(0.0), d, vColor);
//    float edgeFactor = min(min(a3.x, a3.y), a3.z);
//    fAlbedo = vec4(mix(vec3(1.0), fAlbedo.rgb, edgeFactor), fAlbedo.a);

}