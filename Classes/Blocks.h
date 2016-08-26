#pragma once

#include "Stack.h"

#define GLM_SWIZZLE
#include <glm/glm.hpp>

#include <map>
#include <set>
#include <regex>
#include <string>
#include <vector>
#include <utility>
#include <functional>

struct Block {
	bool IsTool        = false;
	bool HasIcon       = false;
	bool IsMBRoot      = false;
	bool Collision     = true;
	bool Craftable     = false;
	bool FullBlock     = true;
	bool Placeable     = true;
    bool Smeltable     = false;
	bool Targetable    = true;
	bool HasTexture    = false;
	bool MultiBlock    = false;
	bool Transparent   = false;
	bool Interactable  = false;
	bool HasCustomData = false;
	bool MultiTextures = false;

	int ID                  = 0;
	int Data                = 0;
	int Icon                = 0;
	int Texture             = 0;
    int Durability          = 0;
	int Luminosity          = 0;
	int MiningLevel         = 0;
    int MiningSpeed         = 0;
	int CraftingYield       = 1;
	int RequiredMiningLevel = 0;
    
    float BurnTime = 0.0f;
	float Hardness = 0.0f;

	std::string Name              = "";
	std::string Sound             = "";
	std::string Material          = "";
	std::string EffectiveMaterial = "";

	glm::vec3  Scale       = {1, 1, 1};
	glm::vec3  ScaleOffset = {0, 0, 0};

	glm::ivec3 MBOffset    = {0, 0, 0};

	std::regex Recipe;
    
    Stack Drop        = Stack();
    Stack SmeltResult = Stack();

	std::function<void()> RightClickFunction;
    std::function<void()> CloseFunction;

    std::vector<int> Textures = {};

    std::vector<std::vector<std::vector<std::pair<glm::vec3, glm::vec3>>>> CustomData = {};

    std::map<int, Block> Types = {};
};

namespace Blocks {
    void Init();
    
    const Block* Get_Block(Stack stack);
    const Block* Get_Block(int type, int data = 0);
    const Block* Get_Block(std::string name);

	const Block* Check_Crafting(std::string grid);

    std::string Get_Name(int type, int data = 0);
    bool Exists(int type, int data = 0);
    bool Exists(std::string name);
    std::vector<float> Mesh(const Block* block, glm::vec3 offset = glm::vec3(0), float scale = 1.0f, std::vector<float> data = {});
    void Mesh(std::vector<float> &storage, const Block* block, glm::vec3 offset = glm::vec3(0), float scale = 1.0f, std::vector<float> data = {}, bool checkMulti = true);
}  // namespace Blocks
