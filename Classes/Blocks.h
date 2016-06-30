#pragma once

#define GLM_SWIZZLE
#include <glm/glm.hpp>

#include <map>
#include <set>
#include <string>
#include <vector>
#include <utility>

struct Block {
    std::string Name = "";
    std::string Sound = "";

    int ID = 0;
    int Data = 0;

    bool FullBlock = true;
    bool Transparent = false;
    bool Collision = true;
    bool Targetable = true;
    bool Placeable = true;

    bool HasIcon = false;
    bool HasTexture = false;
    bool MultiTextures = false;
    bool HasCustomData = false;

    bool IsTool      = false;
    int  Durability  = 0;
    int  MiningSpeed = 0;
    int  MiningLevel = 0;

    int RequiredMiningLevel = 0;

    std::string EffectiveMaterial = "";

    bool       MultiBlock = false;
    bool       IsMBRoot   = false;
    glm::ivec3 MBOffset = glm::ivec3(0);

    float Hardness   = 0;
    int   Luminosity = 0;
    std::string Material = "";

    int Icon    = 0;
    int Texture = 0;

    glm::vec3 Scale = glm::vec3(1);
    glm::vec3 ScaleOffset = glm::vec3(0);

    std::vector<int> Textures = {};
    // Elements -> Side -> Vertex -> (Position, Texture Coords)
    std::vector<std::vector<std::vector<std::pair<glm::vec3, glm::vec3>>>> CustomData = {};

    std::map<int, Block> Types = {};
};

namespace Blocks {
    void Init();

    const Block* Get_Block(int type, int data = 0);
    const Block* Get_Block(std::string name);

    std::string Get_Name(int type, int data = 0);
    bool Exists(int type, int data = 0);
    bool Exists(std::string name);
    std::vector<float> Mesh(const Block* block, glm::vec3 offset = glm::vec3(0), float scale = 1.0f, std::vector<float> data = {});
    void Mesh(std::vector<float> &storage, const Block* block, glm::vec3 offset = glm::vec3(0), float scale = 1.0f, std::vector<float> data = {}, bool checkMulti = true);
}  // namespace Blocks
