#pragma once

#include <map>
#include <set>

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include "VBO.h"

const int CHUNK_SIZE = 16;

class Vec3Comparator {
public:
    bool operator () (const glm::vec3 &a, const glm::vec3 &b) const {
        if (a.x != b.x) return a.x < b.x;
        else if (a.y != b.y) return a.y < b.y;
        else if (a.z != b.z) return a.z < b.z;
        else return false;
    }
};

class Chunk {
public:
    glm::vec3 Position;
    std::set<glm::vec3, Vec3Comparator> Blocks;
    VBO vbo;

    char BlockMap[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE] = {0};
    unsigned char SeesAir[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE] = {0};

    Chunk(glm::vec3 position);

    int GetBlock(glm::vec3 position);
    void SetBlock(glm::vec3 position, char value);

	bool Is_Empty();

	int GetAO(glm::vec3 block, int face, int offset);
	void UpdateAir(glm::ivec3 pos, glm::bvec3 inChunk);

    void Generate();

	bool Check_Grass(glm::vec3 pos);

    void Mesh();

    void RemoveBlock(glm::vec3 position);
	void AddBlock(glm::vec3 position, glm::vec3 diff);
};

std::vector<std::pair<glm::vec3, glm::vec3>> Get_Neighbors(glm::vec3 chunk, glm::vec3 tile);
std::vector<glm::vec3> Get_Chunk_Pos(glm::vec3 worldPos);
glm::vec3 Get_World_Pos(glm::vec3 chunk, glm::vec3 tile);

extern std::map<glm::vec3, Chunk*, Vec3Comparator> ChunkMap;
extern std::map<glm::vec3, std::vector<float>, Vec3Comparator> DataQueue;
extern std::set<glm::vec3, Vec3Comparator> EmptyChunks;