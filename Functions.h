#pragma once

#include <tuple>
#include <glm/glm.hpp>

#include "Variables.h"

glm::vec3 Get_World_Pos(glm::vec3 chunk, glm::vec3 tile) {
    glm::vec3 worldPos;

    worldPos.x = chunk.x * CHUNK_SIZE + tile.x;
    worldPos.y = chunk.y * CHUNK_SIZE + tile.y;
    worldPos.z = chunk.z * CHUNK_SIZE + tile.z;

    return worldPos;
}