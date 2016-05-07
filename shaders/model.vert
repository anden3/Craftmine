#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoords;

out vec2 TexCoords;

layout (std140) uniform Matrices {
    uniform mat4 view;
    uniform mat4 projection;
};

uniform mat4 model;

void main() {
    gl_Position = projection * view * model * vec4(position, 1.0f);
    TexCoords = texCoords;
}