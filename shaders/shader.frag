#version 410 core

struct Material {
    sampler2D diffuse;
};

struct Light {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
};

in vec3 Normal;
in vec2 TexCoords;
in vec3 VertexPos;

out vec4 fragColor;

uniform Light light;
uniform Material material;

uniform bool DrawOutline;
uniform vec3 BlockPos;

void main() {
    float diff = max(dot(Normal, light.direction), 0.0f);

    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));

    vec4 color = vec4(ambient + diffuse, 1.0f);

    if (DrawOutline) {
        vec3 posDiff = VertexPos - BlockPos;

        if (posDiff.x <= 1 && posDiff.x >= 0) {
            if (posDiff.y <= 1 && posDiff.y >= 0) {
                if (posDiff.z <= 1 && posDiff.z >= 0) {
                    color += vec4(0.1f, 0.1f, 0.1f, 0.0f);
                }
            }
        }
    }

    fragColor = color;
}