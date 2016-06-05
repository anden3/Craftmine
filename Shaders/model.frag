#version 410 core

in vec3 TexCoords;

out vec4 FragColor;

uniform sampler2DArray tex;
uniform int lightLevel;

void main() {
    vec4 text = texture(tex, TexCoords);
    FragColor = vec4((0.1f + 0.9f * (lightLevel / 15.0f)) * text.rgb, text.a);
}