#version 450

layout(binding = 0) uniform sampler2D uSampler;

layout(location = 0) in vec3 vColor;
layout(location = 1) in vec2 vTexture;
layout(location = 2) in vec3 vNorm;
layout(location = 3) in vec3 vPosition;

layout(location = 0) out vec4 fAlbedo;
layout(location = 1) out vec4 fNormal;
layout(location = 2) out vec4 fPosition;

void main() {

    fAlbedo = texture(uSampler, vTexture).rgba * vec4(vColor, 1.0f);
    fNormal = vec4(normalize(vNorm), 1);
    fPosition = vec4(vPosition, 1);

//    // ambient lighting
//    float ambient = 0;
//    ambient += (vNorm.x < -0.1) ? 0.03 : 0;
//    ambient += (vNorm.x > +0.1) ? 0.06 : 0;
//    ambient += (vNorm.y < -0.1) ? 0.03 : 0;
//    ambient += (vNorm.y > +0.1) ? 0.08 : 0;
//    ambient += (vNorm.z < -0.1) ? 0.04 : 0;
//    ambient += (vNorm.z > +0.1) ? 0.05 : 0;
//
//    // diffuse lighting
//    vec4 diffuse = vec4(unpackUnorm4x8(uObject.sun.color).rgb * max(dot(fNormal.xyz, uObject.sun.pos), 0.0), 1.0);
//
//    fColor = texture(uSampler, vTexture).rgba * (diffuse + vec4(ambient * 2));

}