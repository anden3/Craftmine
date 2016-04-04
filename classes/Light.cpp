#include "Light.h"

void Light::Add_Dir_Light(Shader shader, glm::vec3 direction, glm::vec3 ambient, glm::vec3 diffuse) {
    glm::vec3 lightDir = glm::normalize(-direction);

    glUniform3f(glGetUniformLocation(shader.Program, "light.direction"), lightDir.x, lightDir.y, lightDir.z);
    glUniform3f(glGetUniformLocation(shader.Program, "light.ambient"),   ambient.x,  ambient.y,  ambient.z);
    glUniform3f(glGetUniformLocation(shader.Program, "light.diffuse"),   diffuse.x,  diffuse.y,  diffuse.z);
}