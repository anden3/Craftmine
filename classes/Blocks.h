#pragma once

#include <map>
#include <string>
#include <vector>

#define GLM_SWIZZLE
#include <glm/glm.hpp>

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
    bool CustomVertices = false;
    
    float Hardness = 0;
    int Luminosity = 0;
    
    int Icon = 0;
    int Texture = 0;
    
    std::vector<int> Textures = {};
    std::vector<std::vector<glm::vec3>> Vertices = {};
    
    std::map<int, Block> Types = {};
};

namespace Blocks {
    void Init();
    
    const Block* Get_Block(unsigned int type, int data = 0);
    const Block* Get_Block_From_Name(std::string name);
    std::string Get_Name(unsigned int type, int data = 0);
    bool Exists(unsigned int type, int data = 0);
}