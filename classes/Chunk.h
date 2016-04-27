#pragma once

#include <map>
#include <set>
#include <queue>
#include <thread>
#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include "VBO.h"

const int CHUNK_SIZE = 16;
const int SUN_LIGHT_LEVEL = 13;

extern unsigned int ChunksQueued;

class Vec3Comparator {
public:
    bool operator () (const glm::vec3 &a, const glm::vec3 &b) const {
        if (a.x != b.x) return a.x < b.x;
        else if (a.z != b.z) return a.z < b.z;
        else if (a.y != b.y) return a.y > b.y;
        else return false;
    }
};

class Vec2Comparator {
public:
    bool operator () (const glm::vec2 &a, const glm::vec2 &b) const {
        if (a.x != b.x) return a.x < b.x;
        else if (a.y != b.y) return a.y < b.y;
        else return false;
    }
};

extern std::map<glm::vec2, std::set<glm::vec2, Vec2Comparator>, Vec2Comparator> topBlocks;

struct LightNode {
    glm::vec3 Chunk;
    glm::vec3 Tile;
    short LightLevel;
    
    LightNode(glm::vec3 chunk, glm::vec3 tile, int lightLevel = 0) {
        Chunk = chunk;
        Tile = tile;
        LightLevel = lightLevel;
    }
};

class Chunk {
public:
    glm::vec3 Position;
    std::set<glm::vec3, Vec3Comparator> Blocks;
    VBO vbo;
    
    bool Generated = false;
    bool Meshed = false;
    bool DataUploaded = false;
    
    std::vector<float> VBOData;

    char BlockMap[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE] = {0};
    unsigned char SeesAir[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE] = {0};
    
    std::queue<LightNode> LightQueue;

    Chunk(glm::vec3 position);
    
    inline int Get_Block(glm::vec3 pos) {
        return BlockMap[int(pos.x)][int(pos.y)][int(pos.z)];
    }
    inline void Set_Block(glm::ivec3 pos, char value) {
        BlockMap[pos.x][pos.y][pos.z] = value;
    }
    
    inline int Get_Air(glm::ivec3 pos) {
        return SeesAir[pos.x][pos.y][pos.z];
    }
    
    void Generate();
    void Light();

	bool Check_Grass(glm::vec3 pos);

    void Mesh();

    void Remove_Block(glm::vec3 position);
	void Add_Block(glm::vec3 position, glm::vec3 diff, int blockType);
    
    inline int Get_Light(glm::vec3 pos) {
        return LightMap[int(pos.x)][int(pos.y)][int(pos.z)];
    }
    inline void Set_Light(glm::ivec3 pos, int value) {
        LightMap[pos.x][pos.y][pos.z] = value;
    }
    
    inline bool Get_Top(glm::vec3 pos) {
        return topBlocks[glm::vec2(Position.x, Position.z)].count(glm::vec2(pos.x, pos.z));
    }
    inline void Set_Top(glm::vec3 pos, bool set) {
        if (set) {
            topBlocks[glm::vec2(Position.x, Position.z)].insert(glm::vec2(pos.x, pos.z));
        }
        else if (Get_Top(pos)) {
            topBlocks[glm::vec2(Position.x, Position.z)].erase(glm::vec2(pos.x, pos.z));
        }
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
extern std::set<glm::vec3, Vec3Comparator> EmptyChunks;

inline bool Exists(glm::vec3 chunk) {
    return ChunkMap.count(chunk) && ChunkMap[chunk]->DataUploaded;
}