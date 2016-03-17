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

out vec4 fragColor;

uniform Light light;
uniform Material material;

void main() {
    float diff = max(dot(Normal, light.direction), 0.0f);

    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));

    fragColor = vec4(ambient + diffuse, 1.0f);
}