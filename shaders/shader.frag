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

void main() {
    vec4 tex = texture(material.diffuse, TexCoords);
    
    float diff = max(dot(Normal, light.direction), 0.0f);
    
    vec3 ambient = light.ambient * tex.rgb;
    vec3 diffuse = light.diffuse * diff * tex.rgb;

    vec4 color = vec4(ambient + diffuse, tex.a);
	vec4 aoAdjustment = vec4(vec3((3 - AO) * 0.05f), 0.0f);

	FragColor = color - aoAdjustment;
}