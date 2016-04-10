#version 410 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;

layout (std140) uniform Matrices {
    uniform mat4 view;
    uniform mat4 projection;
};

uniform mat4 model;

float offset = 0.003;

void main() {
    vec3 position = vertex + (normal * offset);
    gl_Position = projection * view * model * vec4(position, 1.0f);
}