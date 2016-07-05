#pragma once

#include <map>
#include "Comparators.h"

namespace Worlds {
    void Get_Worlds();

    void Save_Chunk(std::string world, glm::vec3 chunkPos);
    std::map<glm::vec3, std::pair<int, int>, VectorComparator> Load_Chunk(std::string world, glm::ivec3 pos);
};