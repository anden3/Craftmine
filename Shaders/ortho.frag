#version 410 core

in vec3 TexCoords;
out vec4 FragColor;

uniform sampler2DArray tex;

void main() {
    FragColor = texture(tex, TexCoords);
}