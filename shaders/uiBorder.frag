#version 410 core

out vec4 FragColor;

uniform vec3 color = vec3(0.0f);

void main() {
    FragColor = vec4(color, 1.0f);
}