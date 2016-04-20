#version 410 core

in vec3 Normal;
in vec2 TexCoords;
in vec3 VertexPos;
in float LightLevel;
in float AO;

out vec4 FragColor;

uniform vec3 ambient;
uniform vec3 diffuse;

uniform sampler2D diffTex;

void main() {
    vec4 tex = texture(diffTex, TexCoords);
    float lightLevel = (LightLevel / 16.0f);
    
    vec3 amb = ambient * tex.rgb;
    vec3 diff = diffuse * tex.rgb * lightLevel;
    
    vec4 color = vec4(amb + diff, tex.a);
    vec4 aoAdjustment = vec4(vec3(AO * 0.08f * lightLevel + 0.01f), 0.0f);
    
    FragColor = color - aoAdjustment;
}