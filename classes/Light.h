#pragma once

#include <glm/glm.hpp>

#include "Shader.h"

namespace Light {
    void Add_Dir_Light(Shader shader, glm::vec3 direction, glm::vec3 ambient, glm::vec3 diffuse);
}