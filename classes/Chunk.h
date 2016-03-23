#pragma once

#include <map>
#include <set>
#include <vector>

#include <glm/glm.hpp>

#include "VBO.h"

const int CHUNK_SIZE = 16;

class Vec3Comparator {
public:
    bool operator () (const glm::vec3 &a, const glm::vec3 &b) const {
        if (a.x != b.x)
            return a.x < b.x;
        else if (a.y != b.y)
            return a.y < b.y;
        else if (a.z != b.z)
            return a.z < b.z;
        else
            return false;
    }
};

class Chunk {
public:
    glm::vec3 Position;
    std::set<glm::vec3, Vec3Comparator> Blocks;
    VBO vbo;

    bool Empty = true;

    char BlockMap[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE] = {0};
    unsigned char SeesAir[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE] = {0};

    Chunk(glm::vec3 position);

    int GetBlock(glm::vec3 position);

    void Generate();
    void Mesh();
};

extern std::map<glm::vec3, Chunk*, Vec3Comparator> ChunkMap;