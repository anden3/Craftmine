#pragma once

#include <set>
#include <queue>
#include <thread>
#include <chrono>

#include "Buffer.h"

const int CHUNK_SIZE = 16;
const int SUN_LIGHT_LEVEL = 15;

extern int IMAGE_SIZE_X;
extern int IMAGE_SIZE_Y;

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

extern std::map<glm::vec2, std::map<glm::vec2, int, Vec2Comparator>, Vec2Comparator> topBlocks;
extern std::map<unsigned int, glm::vec2> textureCoords;
extern std::map<unsigned int, float> blockHardness;

extern std::map<unsigned int, std::vector<glm::vec2>> MultiTextures;
extern std::map<unsigned int, std::vector<std::vector<glm::vec2>>> CustomTexCoords;
extern std::map<unsigned int, std::vector<std::vector<glm::vec3>>> CustomVertices;

extern float vertices[6][6][3];
extern float tex_coords[6][6][2];

struct LightNode {
    glm::vec3 Chunk;
    glm::vec3 Tile;
    short LightLevel;
    bool Down;
    bool Underground;
    
    LightNode(glm::vec3 chunk, glm::vec3 tile, int lightLevel = 0, bool down = false, bool underground = false) {
        Chunk = chunk;
        Tile = tile;
        LightLevel = lightLevel;
        Down = down;
        Underground = underground;
    }
};

class LightNodeComparator {
public:
    bool operator () (const LightNode &a, const LightNode &b) const {
        if (a.Chunk.x != b.Chunk.x) return a.Chunk.x < b.Chunk.x;
        else if (a.Chunk.y != b.Chunk.y) return a.Chunk.y < b.Chunk.y;
        else if (a.Chunk.z != b.Chunk.z) return a.Chunk.z < b.Chunk.z;
        
        else if (a.Tile.x != b.Tile.x) return a.Tile.x < b.Tile.x;
        else if (a.Tile.y != b.Tile.y) return a.Tile.y < b.Tile.y;
        else if (a.Tile.z != b.Tile.z) return a.Tile.z < b.Tile.z;
        
        else return false;
    }
};

class Chunk {
public:
    glm::vec3 Position;
    std::set<glm::vec3, Vec3Comparator> Blocks;
    Buffer buffer;
    
    Data VBOData;
    std::queue<LightNode> LightQueue;
    std::queue<LightNode> LightRemovalQueue;
    
    bool Generated = false;
    bool Meshed = false;
    bool DataUploaded = false;
    bool Visible = true;

    char BlockMap[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE] = {0};
    unsigned char SeesAir[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE] = {0};

    Chunk(glm::vec3 position);
    
    inline int Get_Block(glm::vec3 pos) { return BlockMap[int(pos.x)][int(pos.y)][int(pos.z)]; }
    inline void Set_Block(glm::ivec3 pos, char value) { BlockMap[pos.x][pos.y][pos.z] = value; }
    inline int Get_Air(glm::ivec3 pos) { return SeesAir[pos.x][pos.y][pos.z]; }
    
    void Generate();
    void Light(bool flag = true);
    void Mesh();

    void Remove_Block(glm::ivec3 position);
	void Add_Block(glm::ivec3 position, glm::vec3 diff, int blockType);
    
    inline int Get_Light(glm::vec3 pos) { return LightMap[int(pos.x)][int(pos.y)][int(pos.z)]; }
    inline void Set_Light(glm::ivec3 pos, int value) { LightMap[pos.x][pos.y][pos.z] = value; }
    
    inline bool Get_Top(glm::vec3 pos) { return TopBlocks.count(pos) > 0; }
    inline void Set_Top(glm::vec3 pos, bool set) {
        if (set) {
            TopBlocks.insert(pos);
        }
        else if (Get_Top(pos)) {
            TopBlocks.erase(pos);
        }
    }
    
private:
    void UpdateAir(glm::ivec3 pos, glm::bvec3 inChunk);
    void Check_Ore(glm::ivec3 pos, glm::vec3 noisePos);
    int GetAO(glm::vec3 block, int face, int offset);
        
    unsigned char LightMap[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE] = {0};
    std::set<glm::vec3, Vec3Comparator> TopBlocks;
};

std::vector<std::pair<glm::vec3, glm::vec3>> Get_Neighbors(glm::vec3 chunk, glm::vec3 tile);
std::pair<glm::vec3, glm::vec3> Get_Chunk_Pos(glm::vec3 worldPos);

bool Is_Block(glm::vec3 pos);

inline glm::vec3 Get_World_Pos(glm::vec3 chunk, glm::vec3 tile) {
    chunk *= CHUNK_SIZE;
    return chunk + tile;
}

extern std::map<glm::vec3, Chunk*, Vec3Comparator> ChunkMap;

inline bool Exists(glm::vec3 chunk) {
    return ChunkMap.count(chunk) && ChunkMap[chunk]->DataUploaded;
}