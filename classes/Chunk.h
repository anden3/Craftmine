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
        else if (a.z != b.z) return a.z < b.z;
        else if (a.y != b.y) return a.y > b.y;
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
    unsigned char SunlightMap[CHUNK_SIZE][CHUNK_SIZE] = {0};

    Chunk(glm::vec3 position);
    
    inline int Get_Block(glm::ivec3 pos) {
        return BlockMap[pos.x][pos.y][pos.z];
    }
    
    inline void Set_Block(glm::ivec3 pos, char value) {
        BlockMap[pos.x][pos.y][pos.z] = value;
    }
    
    inline int Get_Air(glm::ivec3 pos) {
        return SeesAir[pos.x][pos.y][pos.z];
    }
    
    void Generate();

	bool Check_Grass(glm::vec3 pos);

    void Mesh();

    void Remove_Block(glm::vec3 position);
	void Add_Block(glm::vec3 position, glm::vec3 diff, int blockType);
    
    inline int Get_Sunlight(glm::ivec3 pos) {
        return (LightMap[pos.x][pos.y][pos.z] >> 4) & 0xF;
    }
    
    inline void Set_Sunlight(glm::ivec3 pos, int value) {
        LightMap[pos.x][pos.y][pos.z] = (LightMap[pos.x][pos.y][pos.z] & 0xF) | (value << 4);
    }
    
    inline int Get_Torchlight(glm::ivec3 pos) {
        return LightMap[pos.x][pos.y][pos.z] & 0xF;
    }
    
    inline void Set_Torchlight(glm::ivec3 pos, int value) {
        LightMap[pos.x][pos.y][pos.z] = (LightMap[pos.x][pos.y][pos.z] & 0xF0) | value;
    }
    
private:
    bool Is_Empty();
    void UpdateAir(glm::ivec3 pos, glm::bvec3 inChunk);
    int GetAO(glm::vec3 block, int face, int offset);
    
    unsigned char LightMap[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE] = {0};
};

std::vector<std::pair<glm::vec3, glm::vec3>> Get_Neighbors(glm::vec3 chunk, glm::vec3 tile);
std::vector<glm::vec3> Get_Chunk_Pos(glm::vec3 worldPos);

inline glm::vec3 Get_World_Pos(glm::vec3 chunk, glm::vec3 tile) {
    chunk *= CHUNK_SIZE;
    return chunk + tile;
}

extern std::map<glm::vec3, Chunk*, Vec3Comparator> ChunkMap;
extern std::map<glm::vec3, std::vector<float>, Vec3Comparator> DataQueue;
extern std::set<glm::vec3, Vec3Comparator> EmptyChunks;