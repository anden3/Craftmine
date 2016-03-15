#version 410 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec3 FragmentPos;
    vec3 NormalVector;
    vec2 TextureCoords;
} gs_in[];

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

void main() {
    gl_Position = gl_in[0].gl_Position;
    FragPos = gs_in[0].FragmentPos;
    Normal = gs_in[0].NormalVector;
    TexCoords = gs_in[0].TextureCoords;
    EmitVertex();

    gl_Position = gl_in[1].gl_Position;
    FragPos = gs_in[1].FragmentPos;
    Normal = gs_in[1].NormalVector;
    TexCoords = gs_in[1].TextureCoords;
    EmitVertex();

    gl_Position = gl_in[2].gl_Position;
    FragPos = gs_in[2].FragmentPos;
    Normal = gs_in[2].NormalVector;
    TexCoords = gs_in[2].TextureCoords;
    EmitVertex();

    EndPrimitive();
}