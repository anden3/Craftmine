#pragma once

#include <map>
#include <string>
#include <vector>

#define GLM_SWIZZLE
#include <glm/glm.hpp>

extern glm::vec2 tex_coords[6][6];
extern glm::vec3 vertices[6][6];

struct Block {
    std::string Name = "";
    std::string Sound = "";
    
    unsigned int ID = 0;
    int Data = 0;
    
    bool FullBlock = true;
    bool Transparent = false;
    bool Collision = true;
    bool Targetable = true;
    bool HasIcon = false;
    bool HasTexture = false;
    bool MultiTextures = false;
    bool HasCustomData = false;
    
    float Hardness = 0;
    int Luminosity = 0;
    
    int Icon = 0;
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
    
    const Block* Get_Block(unsigned int type, int data = 0);
    const Block* Get_Block_From_Name(std::string name);
    std::string Get_Name(unsigned int type, int data = 0);
    bool Exists(unsigned int type, int data = 0);
    std::vector<float> Mesh(const Block* block, glm::vec3 offset = glm::vec3(0), float scale = 1.0f, std::vector<float> data = {});
    void Mesh(std::vector<float> &storage, const Block* block, glm::vec3 offset = glm::vec3(0), float scale = 1.0f, std::vector<float> data = {});
}