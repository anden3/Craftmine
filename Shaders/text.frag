#version 410 core

in vec2 TexCoords;
in vec3 TextColor;

out vec4 FragColor;

uniform sampler2D text;
uniform float Opacity = 1.0f;

void main() {
    FragColor = vec4(TextColor, texture(text, TexCoords).r * Opacity);
}
