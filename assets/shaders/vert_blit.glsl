#version 450

// ?????
// TODO figure out wtf is happening here
// maybe split into two triangles with no off-screen parts (?)

out gl_PerVertex
{
    vec4 gl_Position;
};

layout(location = 0) out vec2 vTexture;

void main()
{
    vTexture = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(vTexture * 2.0f - 1.0f, 0.0f, 1.0f);
}