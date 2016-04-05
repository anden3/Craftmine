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
in float AO;

out vec4 FragColor;

uniform Light light;
uniform Material material;

uniform bool DrawOutline = false;
uniform vec3 BlockPos;

void main() {
    float diff = max(dot(Normal, light.direction), 0.0f);

    vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;

    vec4 color = vec4(ambient + diffuse, 1.0f);

    if (DrawOutline) {
        vec3 posDiff = VertexPos - BlockPos;

        if (posDiff.x <= 1.001 && posDiff.x >= -0.001) {
            if (posDiff.y <= 1.001 && posDiff.y >= -0.001) {
                if (posDiff.z <= 1.001 && posDiff.z >= -0.001) {
                    color += vec4(0.1f, 0.1f, 0.1f, 0.0f);
                }
            }
        }
    }

	vec4 aoAdjustment = vec4(vec3((3 - AO) * 0.05f), 0.0f);

	FragColor = color - aoAdjustment;
}