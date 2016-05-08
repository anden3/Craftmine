#version 410 core

in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec4 textColor;

void main() {
    color = textColor * vec4(1, 1, 1, texture(text, TexCoords).r);
}