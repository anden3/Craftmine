#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in float ao;

out vec3 Normal;
out vec2 TexCoords;
out vec3 VertexPos;
out float AO;

layout (std140) uniform Matrices {
    uniform mat4 view;
    uniform mat4 projection;
};

uniform mat4 model;

void main() {
    gl_Position = projection * view * model * vec4(position, 1.0f);

    VertexPos = vec4(model * vec4(position, 1.0f)).xyz;
    Normal = normal;
    TexCoords = texCoords;
	AO = ao;
}