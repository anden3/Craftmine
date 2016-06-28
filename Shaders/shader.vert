#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 texCoords;
layout (location = 2) in float lightLevel;
layout (location = 3) in float ao;
layout (location = 4) in float extraTexture;

out vec3 TexCoords;
out float LightLevel;
out float AO;
out float ExtraTexture;

layout (std140) uniform Matrices {
    uniform mat4 view;
    uniform mat4 projection;
};

uniform mat4 model;

void main() {
    gl_Position = projection * view * model * vec4(position, 1.0f);
    TexCoords = texCoords;
    LightLevel = lightLevel;
    AO = ao;
    ExtraTexture = extraTexture;
}
