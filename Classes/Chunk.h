#pragma once

#include <set>
#include <array>
#include <queue>
#include <atomic>
#include <thread>

#include "Buffer.h"
#include "Comparators.h"

const int CHUNK_SIZE = 16;

template <class T, size_t... S>
struct ArrayHelper;

template <class T, size_t X, size_t Y, size_t Z>
struct ArrayHelper<T, X, Y, Z> {
    using type = std::array<std::array<std::array<T, Z>, Y>, X>;
};

template <class T, size_t N>
struct ArrayHelper<T, N> {
    using type = std::array<std::array<std::array<T, N>, N>, N>;
};

template<class T, size_t... A>
using Array3D = typename ArrayHelper<T, A...>::type;

extern std::map<glm::vec3, std::map<glm::vec3, std::pair<int, int>, VectorComparator>, ChunkPosComparator> ChangedBlocks;
extern std::map<glm::vec2, std::map<glm::vec2, int, VectorComparator>, VectorComparator> TopBlocks;

namespace Chunks {
    void Seed(int seed);
	void Delete(glm::vec3 chunk);
};

struct Block;

struct LightNode {
    glm::vec3 Chunk;
    glm::vec3 Tile;
    int LightLevel;
    bool Down;

    LightNode(glm::vec3 chunk, glm::vec3 tile, int lightLevel = 0, bool down = false) {
        Chunk = chunk;
        Tile = tile;
        LightLevel = lightLevel;
        Down = down;
    }
};

class Chunk {
public:
    Buffer buffer;
    glm::vec3 Position;

    Data VBOData;
    std::queue<LightNode> LightQueue;
    std::queue<LightNode> LightRemovalQueue;

    std::map<glm::ivec3, int, VectorComparator> ExtraTextures;

	std::atomic_bool Meshed           = ATOMIC_VAR_INIT(false);
	std::atomic_bool Visible          = ATOMIC_VAR_INIT(true);
	std::atomic_bool Generated        = ATOMIC_VAR_INIT(false);
	std::atomic_bool DataUploaded     = ATOMIC_VAR_INIT(false);
	std::atomic_bool HasExtraTextures = ATOMIC_VAR_INIT(false);

    Chunk(glm::vec3 position) {
        Position = position;
    }

    inline int Get_Type(glm::uvec3 pos) { return BlockMap[pos.x][pos.y][pos.z]; }
    inline void Set_Type(glm::uvec3 pos, int value) { BlockMap[pos.x][pos.y][pos.z] = value; }
    inline unsigned char Get_Air(glm::uvec3 pos) { return SeesAir[pos.x][pos.y][pos.z]; }
    inline unsigned char& Get_Air_Ref(glm::uvec3 pos) { return SeesAir[pos.x][pos.y][pos.z]; }

    inline int Get_Data(glm::vec3 pos) { return DataMap.count(pos) ? DataMap[pos] : 0; }
    inline void Set_Data(glm::vec3 pos, int data) { DataMap[pos] = data; }

    void Generate();
    void Light(bool flag = true);
    void Mesh();
    void Draw(bool transparentPass = false);

    void Remove_Multiblock(glm::ivec3 position, const Block* block);
    void Add_Multiblock(glm::ivec3 position, const Block* block);

    void Remove_Block(glm::ivec3 position, bool checkMulti = true);
    void Add_Block(glm::ivec3 position, int blockType, int blockData, bool checkMulti = true);

    inline int Get_Light(glm::uvec3 pos) {
        return LightMap[pos.x][pos.y][pos.z];
    }
    inline void Set_Light(glm::uvec3 pos, int value) {
        LightMap[pos.x][pos.y][pos.z] = static_cast<unsigned char>(value);
    }

    inline bool Top_Exists(glm::ivec3 tile) {
        return TopBlocks.count(Position.xz()) && TopBlocks[Position.xz()].count(tile.xz());
    }
    inline int Get_Top(glm::ivec3 tile) {
        return TopBlocks[Position.xz()][tile.xz()];
    }
    inline void Set_Top(glm::ivec3 tile, int value) {
        TopBlocks[Position.xz()][tile.xz()] = value;
    }
private:
    bool ContainsTransparentBlocks = false;

    void Update_Air(glm::ivec3 pos, glm::bvec3 inChunk);
    void Update_Transparency(glm::ivec3 pos);

    void Generate_Tree(glm::vec3 tile);
    void Check_Ore(glm::ivec3 pos, glm::dvec3 noisePos);

    float GetAO(glm::vec3 block, int face, int offset);
    int Get_Extra_Texture(glm::ivec3 tile);

    Array3D<int, CHUNK_SIZE>           BlockMap = {0};
    Array3D<unsigned char, CHUNK_SIZE> LightMap = {0};
    Array3D<unsigned char, CHUNK_SIZE> SeesAir  = {0};

    std::set<glm::vec3, VectorComparator> Blocks;
    std::map<glm::vec3, int, VectorComparator> DataMap;
    std::set<glm::vec3, VectorComparator> TransparentBlocks;
};

std::vector<std::pair<glm::vec3, glm::vec3>> Get_Neighbors(glm::vec3 chunk, glm::vec3 tile);
std::pair<glm::vec3, glm::vec3> Get_Chunk_Pos(glm::vec3 worldPos);

bool Is_Block(glm::vec3 pos);
bool Exists(glm::vec3 chunk);

inline glm::vec3 Get_World_Pos(glm::vec3 chunk, glm::vec3 tile) {
	return glm::fma(glm::vec3(static_cast<float>(CHUNK_SIZE)), chunk, tile);
}
