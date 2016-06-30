#version 410 core

layout (location = 0) in vec2 vertex;
layout (location = 1) in vec2 texCoords;
layout (location = 2) in vec3 textColor;

out vec2 TexCoords;
out vec3 TextColor;

uniform mat4 projection;
uniform vec2 Position;

void main() {
    gl_Position = projection * vec4(vertex + Position, 0, 1);
    TexCoords = texCoords;
    TextColor = textColor;
}
