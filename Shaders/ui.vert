#version 410 core

layout (location = 0) in vec2 vertex;

uniform mat4 projection;
uniform mat4 model;

void main() {
    gl_Position = projection * model * vec4(vertex, 0, 1);
}
