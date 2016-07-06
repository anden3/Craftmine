#pragma once

#include <map>
#include <string>
#include <vector>

#include "Comparators.h"

struct World {
    World(std::string name, int seed) : Name(name), Seed(seed) {}

    std::string Name;
    int Seed;
};

namespace Worlds {
    std::string Get_Name(int seed);
    int Get_Seed(std::string name);

    void Create_World(std::string name, int seed);
    void Delete_World(std::string name);

    void Save_World();
    void Load_World(int seed);

    std::vector<World> Get_Worlds();

    void Save_Chunk(std::string world, glm::vec3 chunkPos);
    std::map<glm::vec3, std::pair<int, int>, VectorComparator> Load_Chunk(std::string world, glm::ivec3 pos);
};