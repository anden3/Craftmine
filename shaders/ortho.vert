#version 410 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec2 texCoords;
layout (location = 2) in vec2 offset;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 projection;

uniform float scale;

void main() {
    vec3 Vertex = vertex * scale;
    
    gl_Position = projection * (model * vec4(Vertex, 1.0f) + vec4(offset, 0.0f, 1.0f));
    TexCoords = texCoords;
}