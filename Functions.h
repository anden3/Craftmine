#pragma once

#include "Variables.h"
#include <tuple>

glm::vec3 Get_World_Pos(glm::vec3 chunk, glm::vec3 tile) {
    glm::vec3 worldPos;

    worldPos.x = chunk.x * CHUNK_SIZE + tile.x;
    worldPos.y = chunk.y * CHUNK_SIZE + tile.y;
    worldPos.z = chunk.z * CHUNK_SIZE + tile.z;

    return worldPos;
}

std::tuple<glm::vec3, glm::vec3> Get_Chunk_Pos(glm::vec3 worldPos) {
    glm::vec3 chunk((int) (worldPos.x / CHUNK_SIZE), (int) (worldPos.y / CHUNK_SIZE), (int) (worldPos.z / CHUNK_SIZE));
    glm::vec3 tile((int) worldPos.x % (int) CHUNK_SIZE, (int) worldPos.y % (int) CHUNK_SIZE, (int) worldPos.z % (int) CHUNK_SIZE);

    return std::make_tuple(chunk, tile);
}