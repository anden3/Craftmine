#version 410 core

in vec3 TexCoords;
in float LightLevel;
in float AO;
in float ExtraTexture;

out vec4 FragColor;

uniform vec3 ambient;
uniform vec3 diffuse;

uniform sampler2DArray diffTex;
uniform bool RenderTransparent = false;

void main() {
    vec4 tex = texture(diffTex, TexCoords);

    if (ExtraTexture > 0) {
        vec4 tex2 = texture(diffTex, vec3(TexCoords.xy, ExtraTexture));
        tex = vec4(tex.rgb * (1 - (0.5f * tex2.a)), tex.a);
    }

    if ((RenderTransparent && tex.a == 1.0f) || (!RenderTransparent && tex.a < 1.0f)) {
        discard;
    }

    float lightLevel = ((LightLevel + 1) / 16.0f);

    vec3 amb = ambient * tex.rgb;
    vec3 diff = diffuse * tex.rgb * lightLevel;

    vec4 color = vec4(diff, tex.a);
    vec4 aoAdjustment = vec4(vec3(AO * 0.08f * lightLevel + 0.01f), 0.0f);
    FragColor = color - aoAdjustment;
}
