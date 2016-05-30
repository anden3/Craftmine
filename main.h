#pragma once

#include <map>
#include <set>
#include <vector>
#include <string>

#define GLM_SWIZZLE
#include <glm/glm.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#ifdef _WIN32
const bool Windows = true;

#elif __APPLE__
const bool Windows = false;

#endif

// TODO: Add 2D icon functionality to inventory.
// TODO: Sort rendering of transparent objects.
// TODO: Fix water borders.

const glm::vec3 CLEAR_COLOR = glm::vec3(0.529f, 0.808f, 0.922f);
const glm::vec3 AMBIENT_LIGHT = glm::vec3(0.1f);
const glm::vec3 DIFFUSE_LIGHT = glm::vec3(0.7f);

const std::string CONFIG_FILE = "config.conf";

glm::vec3 vertices[6][6] = {
    { {0, 0, 0}, {0, 1, 1}, {0, 1, 0}, {0, 1, 1}, {0, 0, 0}, {0, 0, 1} },
    { {1, 0, 0}, {1, 1, 0}, {1, 1, 1}, {1, 0, 0}, {1, 1, 1}, {1, 0, 1} },
    { {0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {1, 0, 1}, {0, 0, 1}, {0, 0, 0} },
    { {0, 1, 0}, {1, 1, 1}, {1, 1, 0}, {0, 1, 0}, {0, 1, 1}, {1, 1, 1} },
    { {0, 0, 0}, {1, 1, 0}, {1, 0, 0}, {0, 0, 0}, {0, 1, 0}, {1, 1, 0} },
    { {0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {1, 1, 1}, {0, 1, 1}, {0, 0, 1} },
};

glm::vec2 tex_coords[6][6] = {
    { {0, 1}, {1, 0}, {0, 0}, {1, 0}, {0, 1}, {1, 1} },
    { {1, 1}, {1, 0}, {0, 0}, {1, 1}, {0, 0}, {0, 1} },
    { {0, 0}, {1, 0}, {1, 1}, {1, 1}, {0, 1}, {0, 0} },
    { {1, 0}, {0, 1}, {1, 1}, {1, 0}, {0, 0}, {0, 1} },
    { {1, 1}, {0, 0}, {0, 1}, {1, 1}, {1, 0}, {0, 0} },
    { {0, 1}, {1, 1}, {1, 0}, {1, 0}, {0, 0}, {0, 1} }
};

struct Block;
std::map<unsigned int, Block> BlockTypes;

std::map<unsigned int, glm::vec2> textureCoords = {
    // BLOCKS        
    {37,  {14,  1}}, // Dandelion
    {38,  {15,  1}}, // Poppy
    {39,  {15,  2}}, // Brown Mushroom
    {40,  {14,  2}}, // Red Mushroom
    {41,  { 8,  2}}, // Gold Block
    {42,  { 7,  2}}, // Iron Block
    {43,  { 6,  1}}, // Double Stone Slab
    {45,  { 8,  1}}, // Bricks
    {46,  { 9,  1}}, // TNT
    {47,  { 4,  3}}, // Bookshelf
    {48,  { 5,  3}}, // Mossy Cobblestone
    {49,  { 6,  3}}, // Obsidian
    
    {52,  { 2,  5}}, // Monster Spawner
    
    {54,  {12,  2}}, // Chest
    
    {56,  { 3,  4}}, // Diamond Ore
    {57,  { 9,  2}}, // Diamond Block
    {58,  {13,  4}}, // Crafting Table
    
    {60,  { 8,  6}}, // Farmland
    {61,  {13,  3}}, // Furnace
    {62,  {14,  4}}, // Burning Furnace
    
    {64,  {12, 19}}, // Oak Door
    {65,  { 4,  6}}, // Ladder
    {66,  { 1,  9}}, // Rail
    
    {69,  { 1,  7}}, // Lever
    {71,  {13, 19}}, // Iron Door
    {73,  { 4,  4}}, // Redstone Ore
    
    {75,  { 4,  8}}, // Redstone Torch (off)
    {76,  { 4,  7}}, // Redstone Torch (on)
    
    {78,  { 3,  5}}, // Snow (layer)
    {79,  { 4,  5}}, // Ice
    {80,  { 3,  5}}, // Snow (block)
    {81,  { 7,  5}}, // Cactus
    {82,  { 9,  3}}, // Clay
    
    // ITEMS
    {256, { 3, 22}}, // Iron Shovel
    {257, { 3, 23}}, // Iron Pickaxe
    {258, { 3, 24}}, // Iron Axe
    {259, { 6, 17}}, // Flint and Steel
    {260, {11, 17}}, // Apple
    {261, { 6, 18}}, // Bow
    {262, { 6, 19}}, // Arrow
    {263, { 8, 17}}, // Coal
    {264, { 8, 20}}, // Diamond
    {265, { 8, 18}}, // Iron Ingot
    {266, { 8, 19}}, // Gold Ingot
    
    {280, { 6, 20}}, // Stick
};

std::map<unsigned int, std::vector<glm::vec2>> MultiTextures = {
    //      LEFT      RIGHT     DOWN      UP        BACK      FRONT
    {46, {{ 9,  1}, { 9,  1}, {11,  1}, {10,  1}, { 9,  1}, { 9,  1}}}, // TNT
    {47, {{ 4,  3}, { 4,  3}, { 5,  1}, { 5,  1}, { 4,  3}, { 4,  3}}}, // Bookshelf
    {54, {{11,  2}, {11,  2}, {10,  2}, {10,  2}, {11,  2}, {12,  2}}}, // Chest
    {58, {{12,  4}, {12,  4}, { 5,  1}, {12,  3}, {13,  4}, {13,  4}}}, // Crafting Table
    {60, {{ 3,  1}, { 3,  1}, { 3,  1}, { 8,  6}, { 3,  1}, { 3,  1}}}, // Farmland
    {61, {{14,  3}, {14,  3}, {15,  4}, {15,  4}, {14,  3}, {13,  3}}}, // Furnace
    {62, {{14,  3}, {14,  3}, {15,  4}, {15,  4}, {14,  3}, {14,  4}}}, // Burning Furnace
    {81, {{ 7,  5}, { 7,  5}, { 6,  5}, { 6,  5}, { 7,  5}, { 7,  5}}}, // Cactus
};

std::map<std::string, std::vector<unsigned int>> BlockSounds = {
    {"dirt",   {60}},
    {"sand",   {82}},
    {"snow",   {78, 80}},
    {"stone",  {21, 22, 23, 24, 41, 42, 43, 44, 45, 48, 52, 56, 57, 61, 62, 73}},
    {"wood",   {25, 47, 49, 54, 58, 65}}
};

std::map<std::string, std::vector<std::vector<glm::vec2>>> PlayerTexCoords = {
    {"head", { {{16,  8}, {24, 16}}, {{0,   8}, {8,  16}}, {{16,  0}, {24,  8}}, {{8,   0}, {16,  8}}, {{8,   8}, {16, 16}}, {{24,  8}, {32, 16}} }},
    {"body", { {{28, 20}, {32, 32}}, {{16, 20}, {20, 32}}, {{28, 16}, {36, 20}}, {{20, 16}, {28, 20}}, {{20, 20}, {28, 32}}, {{32, 20}, {40, 32}} }},
    {"leg",  { {{8,  20}, {12, 32}}, {{0,  20}, {4,  32}}, {{8,  16}, {12, 20}}, {{4,  16}, {8,  20}}, {{4,  20}, {8,  32}}, {{12, 20}, {16, 32}} }},
    {"arm",  { {{48, 20}, {52, 32}}, {{40, 20}, {40, 32}}, {{48, 16}, {52, 20}}, {{44, 16}, {48, 20}}, {{44, 20}, {48, 32}}, {{52, 20}, {56, 32}} }},
};

int Fullscreen;
int SCREEN_WIDTH, SCREEN_HEIGHT;
int VSync;

int RenderDistance;

std::map<std::string, int*> Options = {
    {"RenderDistance", &RenderDistance},
    {"FullScreen", &Fullscreen},
    {"WindowResX", &SCREEN_WIDTH},
    {"WindowResY", &SCREEN_HEIGHT},
    {"VSync", &VSync}
};

glm::vec2 IMAGE_SIZE = glm::vec2(16, 32);

double DeltaTime = 0.0;
double LastFrame = 0.0;

bool Wireframe = false;
bool ToggleWireframe = false;

bool GamePaused = true;
bool MouseEnabled = false;

bool ChunkMapBusy = false;