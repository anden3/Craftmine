#pragma once

#include "Shader.h"
#include "UI.h"

#ifdef _WIN32
const bool Windows = true;

#elif __APPLE__
const bool Windows = false;

#endif

const glm::vec3 CLEAR_COLOR = glm::vec3(0.529f, 0.808f, 0.922f);
const glm::vec3 AMBIENT_LIGHT = glm::vec3(0.1f);
const glm::vec3 DIFFUSE_LIGHT = glm::vec3(0.1f);

const std::string CONFIG_FILE = "config.conf";

typedef std::vector<glm::vec2> TexArray;
typedef std::vector<glm::vec3> VertexArray;

typedef glm::vec2 Coord;
typedef glm::vec3 Vertex;

float vertices[6][6][3] = {
    { {0, 0, 0}, {0, 1, 1}, {0, 1, 0}, {0, 1, 1}, {0, 0, 0}, {0, 0, 1} },
    { {1, 0, 0}, {1, 1, 0}, {1, 1, 1}, {1, 0, 0}, {1, 1, 1}, {1, 0, 1} },
    { {0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {1, 0, 1}, {0, 0, 1}, {0, 0, 0} },
    { {0, 1, 0}, {1, 1, 1}, {1, 1, 0}, {0, 1, 0}, {0, 1, 1}, {1, 1, 1} },
    { {0, 0, 0}, {1, 1, 0}, {1, 0, 0}, {0, 0, 0}, {0, 1, 0}, {1, 1, 0} },
    { {0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {1, 1, 1}, {0, 1, 1}, {0, 0, 1} }
};

float tex_coords[6][6][2] = {
    { {0, 1}, {1, 0}, {0, 0}, {1, 0}, {0, 1}, {1, 1} },
    { {1, 1}, {1, 0}, {0, 0}, {1, 1}, {0, 0}, {0, 1} },
    { {0, 0}, {1, 0}, {1, 1}, {1, 1}, {0, 1}, {0, 0} },
    { {1, 0}, {0, 1}, {1, 1}, {1, 0}, {0, 0}, {0, 1} },
    { {1, 1}, {0, 0}, {0, 1}, {1, 1}, {1, 0}, {0, 0} },
    { {0, 1}, {1, 1}, {1, 0}, {1, 0}, {0, 0}, {0, 1} }
};

std::map<unsigned int, glm::vec2> textureCoords = {
    // BLOCKS
    {1,   Coord( 2,  1)}, // Stone
    {2,   Coord( 1,  1)}, // Grass Top
    {3,   Coord( 3,  1)}, // Dirt
    {4,   Coord( 1,  2)}, // Cobblestone
    {5,   Coord( 5,  1)}, // Wooden Planks
    {6,   Coord(16,  1)}, // Oak Sapling
    {7,   Coord( 2,  2)}, // Bedrock
    {8,   Coord(14, 13)}, // Flowing Water
    {9,   Coord(14, 13)}, // Still Water
    {10,  Coord(14, 15)}, // Flowing Lava
    {11,  Coord(14, 15)}, // Still Lava
    {12,  Coord( 3,  2)}, // Sand
    {13,  Coord( 4,  2)}, // Gravel
    {14,  Coord( 1,  3)}, // Gold Ore
    {15,  Coord( 2,  3)}, // Iron Ore
    {16,  Coord( 3,  3)}, // Coal Ore
    {17,  Coord( 5,  2)}, // Oak Log
    {18,  Coord( 5,  4)}, // Oak Leaves
    {19,  Coord( 1,  4)}, // Sponge
    {20,  Coord( 2,  4)}, // Glass
    {21,  Coord( 1, 11)}, // Lapis Lazuli Ore
    {22,  Coord( 1, 10)}, // Lapis Lazuli Block
    {23,  Coord(15,  3)}, // Dispenser
    {24,  Coord( 1, 13)}, // Sandstone
    {25,  Coord(11,  5)}, // Note Block
    {26,  Coord(14, 19)}, // Bed
    {27,  Coord( 4, 12)}, // Powered Rail
    
    {30,  Coord(12,  1)}, // Cobweb
    
    {35,  Coord( 1,  5)}, // White Wool
    {37,  Coord(14,  1)}, // Dandelion
    {38,  Coord(15,  1)}, // Poppy
    {39,  Coord(15,  2)}, // Brown Mushroom
    {40,  Coord(14,  2)}, // Red Mushroom
    {41,  Coord( 8,  2)}, // Gold Block
    {42,  Coord( 7,  2)}, // Iron Block
    {43,  Coord( 6,  1)}, // Double Stone Slab
    {44,  Coord( 7,  1)}, // Stone Slab
    {45,  Coord( 8,  1)}, // Bricks
    {46,  Coord( 9,  1)}, // TNT
    {47,  Coord( 4,  3)}, // Bookshelf
    {48,  Coord( 5,  3)}, // Mossy Cobblestone
    {49,  Coord( 6,  3)}, // Obsidian
    {50,  Coord( 1,  6)}, // Torch
    
    {52,  Coord( 2,  5)}, // Monster Spawner
    
    {54,  Coord(12,  2)}, // Chest
    
    {56,  Coord( 3,  4)}, // Diamond Ore
    {57,  Coord( 9,  2)}, // Diamond Block
    {58,  Coord(13,  4)}, // Crafting Table
    
    {60,  Coord( 8,  6)}, // Farmland
    {61,  Coord(13,  3)}, // Furnace
    {62,  Coord(14,  4)}, // Burning Furnace
    
    {64,  Coord(12, 19)}, // Oak Door
    {65,  Coord( 4,  6)}, // Ladder
    {66,  Coord( 1,  9)}, // Rail
    
    {69,  Coord( 1,  7)}, // Lever
    {70,  Coord( 2,  1)}, // Stone Pressure Plate
    {71,  Coord(13, 19)}, // Iron Door
    {72,  Coord( 5,  1)}, // Wood Pressure Plate
    {73,  Coord( 4,  4)}, // Redstone Ore
    
    {75,  Coord( 4,  8)}, // Redstone Torch (off)
    {76,  Coord( 4,  7)}, // Redstone Torch (on)
    
    {78,  Coord( 3,  5)}, // Snow (layer)
    {79,  Coord( 4,  5)}, // Ice
    {80,  Coord( 3,  5)}, // Snow (block)
    {81,  Coord( 7,  5)}, // Cactus
    {82,  Coord( 9,  3)}, // Clay
    
    // Damage Textures
    {246, Coord( 1, 16)},
    {247, Coord( 2, 16)},
    {248, Coord( 3, 16)},
    {249, Coord( 4, 16)},
    {250, Coord( 5, 16)},
    {251, Coord( 6, 16)},
    {252, Coord( 7, 16)},
    {253, Coord( 8, 16)},
    {254, Coord( 9, 16)},
    {255, Coord(10, 16)},
    
    // ITEMS
    {256, Coord( 3, 22)}, // Iron Shovel
    {257, Coord( 3, 23)}, // Iron Pickaxe
    {258, Coord( 3, 24)}, // Iron Axe
    {259, Coord( 6, 17)}, // Flint and Steel
    {260, Coord(11, 17)}, // Apple
    {261, Coord( 6, 18)}, // Bow
    {262, Coord( 6, 19)}, // Arrow
    {263, Coord( 8, 17)}, // Coal
    {264, Coord( 8, 20)}, // Diamond
    {265, Coord( 8, 18)}, // Iron Ingot
    {266, Coord( 8, 19)}, // Gold Ingot
    
    {280, Coord( 6, 20)}, // Stick
};

std::map<unsigned int, TexArray> MultiTextures = {
    //             LEFT           RIGHT          DOWN           UP             BACK           FRONT
    {2,  TexArray {Coord( 4,  1), Coord( 4,  1), Coord( 3,  1), Coord( 1,  1), Coord( 4,  1), Coord( 4,  1)}}, // Grass
    {17, TexArray {Coord( 5,  2), Coord( 5,  2), Coord( 6,  2), Coord( 6,  2), Coord( 5,  2), Coord( 5,  2)}}, // Oak Log
    {23, TexArray {Coord(14,  3), Coord(14,  3), Coord(15,  4), Coord(15,  4), Coord(14,  3), Coord(15,  3)}}, // Dispenser
    {24, TexArray {Coord( 1, 13), Coord( 1, 13), Coord( 1, 12), Coord( 1, 12), Coord( 1, 13), Coord( 1, 13)}}, // Sandstone
    {46, TexArray {Coord( 9,  1), Coord( 9,  1), Coord(11,  1), Coord(10,  1), Coord( 9,  1), Coord( 9,  1)}}, // TNT
    {47, TexArray {Coord( 4,  3), Coord( 4,  3), Coord(5 ,  1), Coord(5 ,  1), Coord( 4,  3), Coord( 4,  3)}}, // Bookshelf
    {54, TexArray {Coord(11,  2), Coord(11,  2), Coord(10,  2), Coord(10,  2), Coord(11,  2), Coord(12,  2)}}, // Chest
    {58, TexArray {Coord(12,  4), Coord(12,  4), Coord( 5,  1), Coord(12,  3), Coord(13,  4), Coord(13,  4)}}, // Crafting Table
    {60, TexArray {Coord( 3,  1), Coord( 3,  1), Coord( 3,  1), Coord( 8,  6), Coord( 3,  1), Coord( 3,  1)}}, // Farmland
    {61, TexArray {Coord(14,  3), Coord(14,  3), Coord(15,  4), Coord(15,  4), Coord(14,  3), Coord(13,  3)}}, // Furnace
    {62, TexArray {Coord(14,  3), Coord(14,  3), Coord(15,  4), Coord(15,  4), Coord(14,  3), Coord(14,  4)}}, // Burning Furnace
    {81, TexArray {Coord( 7,  5), Coord( 7,  5), Coord( 6,  5), Coord( 6,  5), Coord( 7,  5), Coord( 7,  5)}}, // Cactus
};

std::map<unsigned int, std::vector<TexArray>> CustomTexCoords = {
    {26, { // Bed
        TexArray {Coord(8, 9.5), Coord(6, 10)},
        TexArray {Coord(6, 9.5), Coord(8, 10)},
        TexArray {Coord(4, 0),   Coord(5, 1)},
        TexArray {Coord(6, 8),   Coord(8, 9)},
        TexArray {Coord(8, 9.5), Coord(9, 10)},
        TexArray {Coord(5, 9.5), Coord(6, 10)},
    }},
    
    {44, { // Stone Slab
        TexArray {Coord(5, 0), Coord(6, 0.5)},
        TexArray {Coord(5, 0), Coord(6, 0.5)},
        TexArray {Coord(6, 0), Coord(7, 1)},
        TexArray {Coord(6, 0), Coord(7, 1)},
        TexArray {Coord(5, 0), Coord(6, 0.5)},
        TexArray {Coord(5, 0), Coord(6, 0.5)},
    }},
    
    {50, { // Torch
        TexArray {Coord(0.4375, 5.375), Coord(0.5625, 6)},
        TexArray {Coord(0.4375, 5.375), Coord(0.5625, 6)},
        TexArray {Coord(5,      1),     Coord(6,      2)},
        TexArray {Coord(7,      1),     Coord(8,      2)},
        TexArray {Coord(0.4375, 5.375), Coord(0.5625, 6)},
        TexArray {Coord(0.4375, 5.375), Coord(0.5625, 6)},
    }},
    
    {64, { // Oak Door
        TexArray {Coord(1, 5),      Coord(1.0625, 7)},
        TexArray {Coord(1.9375, 5), Coord(2, 7)},
        TexArray {Coord(1, 6.9375), Coord(2, 7)},
        TexArray {Coord(1, 5),      Coord(2, 5.0625)},
        TexArray {Coord(2, 5),      Coord(1, 7)},
        TexArray {Coord(1, 5),      Coord(2, 7)},
    }},
    
    {71, { // Iron Door
        TexArray {Coord(2, 5),      Coord(2.0625, 7)},
        TexArray {Coord(2.9375, 5), Coord(3, 7)},
        TexArray {Coord(2, 6.9375), Coord(3, 7)},
        TexArray {Coord(2, 5),      Coord(3, 5.0625)},
        TexArray {Coord(3, 5),      Coord(2, 7)},
        TexArray {Coord(2, 5),      Coord(3, 7)},
    }},
};

std::map<unsigned int, std::vector<VertexArray>> CustomVertices = {
    {26, { // Bed
        VertexArray {Vertex(0, 0,   0),     Vertex(0, 0.5, 2)},
        VertexArray {Vertex(1, 0,   0),     Vertex(1, 0.5, 2)},
        VertexArray {Vertex(0, 0,   0),     Vertex(1, 0,   2)},
        VertexArray {Vertex(0, 0.5, 0),     Vertex(1, 0.5, 2)},
        VertexArray {Vertex(0, 0,   0),     Vertex(1, 0.5, 0)},
        VertexArray {Vertex(0, 0,   2),     Vertex(1, 0.5, 2)},
    }},
    
    {44, { // Stone Slab
        VertexArray {Vertex(0, 0,   0),     Vertex(0, 0.5, 1)},
        VertexArray {Vertex(1, 0,   0),     Vertex(1, 0.5, 1)},
        VertexArray {Vertex(0, 0,   0),     Vertex(1, 0,   1)},
        VertexArray {Vertex(0, 0.5, 0),     Vertex(1, 0.5, 1)},
        VertexArray {Vertex(0, 0,   0),     Vertex(1, 0.5, 0)},
        VertexArray {Vertex(0, 0,   1),     Vertex(1, 0.5, 1)},
    }},
    
    {50, { // Torch
        VertexArray {Vertex(0.4375, 0,     0.4375),     Vertex(0.5625, 0.625, 0.5625)},
        VertexArray {Vertex(0.4375, 0,     0.4375),     Vertex(0.5625, 0.625, 0.5625)},
        VertexArray {Vertex(0.4375, 0.625, 0.4375),     Vertex(0.5625, 0.625, 0.5625)},
        VertexArray {Vertex(0.4375, 0.625, 0.4375),     Vertex(0.5625, 0.625, 0.5625)},
        VertexArray {Vertex(0.4375, 0,     0.4375),     Vertex(0.5625, 0.625, 0.5625)},
        VertexArray {Vertex(0.4375, 0,     0.4375),     Vertex(0.5625, 0.625, 0.5625)},
    }},
    
    {64, { // Oak Door
        VertexArray {Vertex(0, 0,   0.4375),     Vertex(0, 2, 0.5625)},
        VertexArray {Vertex(1, 0,   0.4375),     Vertex(1, 2, 0.5625)},
        VertexArray {Vertex(0, 0,   0.4375),     Vertex(1, 0, 0.5625)},
        VertexArray {Vertex(0, 2,   0.4375),     Vertex(1, 2, 0.5625)},
        VertexArray {Vertex(0, 0,   0.4375),     Vertex(1, 2, 0.4375)},
        VertexArray {Vertex(0, 0,   0.5625),     Vertex(1, 2, 0.5625)},
    }},
    
    {70, { // Stone Pressure Plate
        VertexArray {Vertex(0, 0,   0),     Vertex(0, 0.1, 1)},
        VertexArray {Vertex(1, 0,   0),     Vertex(1, 0.1, 1)},
        VertexArray {Vertex(0, 0,   0),     Vertex(1, 0,   1)},
        VertexArray {Vertex(0, 0.1, 0),     Vertex(1, 0.1, 1)},
        VertexArray {Vertex(0, 0,   0),     Vertex(1, 0.1, 0)},
        VertexArray {Vertex(0, 0,   1),     Vertex(1, 0.1, 1)},
    }},
    
    {71, { // Iron Door
        VertexArray {Vertex(0, 0,   0.4375),     Vertex(0, 2, 0.5625)},
        VertexArray {Vertex(1, 0,   0.4375),     Vertex(1, 2, 0.5625)},
        VertexArray {Vertex(0, 0,   0.4375),     Vertex(1, 0, 0.5625)},
        VertexArray {Vertex(0, 2,   0.4375),     Vertex(1, 2, 0.5625)},
        VertexArray {Vertex(0, 0,   0.4375),     Vertex(1, 2, 0.4375)},
        VertexArray {Vertex(0, 0,   0.5625),     Vertex(1, 2, 0.5625)},
    }},
    
    {72, { // Wood Pressure Plate
        VertexArray {Vertex(0, 0,   0),     Vertex(0, 0.1, 1)},
        VertexArray {Vertex(1, 0,   0),     Vertex(1, 0.1, 1)},
        VertexArray {Vertex(0, 0,   0),     Vertex(1, 0,   1)},
        VertexArray {Vertex(0, 0.1, 0),     Vertex(1, 0.1, 1)},
        VertexArray {Vertex(0, 0,   0),     Vertex(1, 0.1, 0)},
        VertexArray {Vertex(0, 0,   1),     Vertex(1, 0.1, 1)},
    }},
};

std::map<unsigned int, float> blockHardness = {
    {1, 1.5f},
    {2, 0.5f},
    {3, 0.5f},
    {4, 2.0f},
    {5, 2.0f},
    {6, 0.0f},
};

std::map<std::string, std::vector<unsigned int>> BlockSounds = {
    {"cloth",  std::vector<unsigned int> {35}},
    {"dirt",   std::vector<unsigned int> {2, 3, 60}},
    {"gravel", std::vector<unsigned int> {13}},
    {"sand",   std::vector<unsigned int> {12, 82}},
    {"snow",   std::vector<unsigned int> {78, 80}},
    {"stone",  std::vector<unsigned int> {1, 4, 7, 14, 15, 16, 21, 22, 23, 24, 41, 42, 43, 44, 45, 48, 52, 56, 57, 61, 62, 70, 73}},
    {"wood",   std::vector<unsigned int> {5, 25, 26, 47, 49, 54, 58, 64, 65, 72}}
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

unsigned int DebugVBO, DebugVAO;
unsigned int OutlineVBO, OutlineVAO;
unsigned int UBO;

unsigned int IMAGE_SIZE = 0;

double DeltaTime = 0.0;
double LastFrame = 0.0;

bool Wireframe = false;
bool ToggleWireframe = false;

bool gamePaused = false;
bool MouseEnabled = false;

bool ChunkMapBusy = false;
bool SunlightQueueBusy = false;

Player player = Player();
Chat chat = Chat();
Inventory inventory = Inventory();

Shader* shader;
Shader* outlineShader;
Shader* modelShader;

int modelMatrixLocation;
int diffuseTextureLocation;

GLFWwindow* Window;

std::map<glm::vec3, Chunk*, Vec3Comparator> ChunkMap;

void Parse_Config();

void Init_GL();
void Init_Textures();
void Init_Shaders();
void Init_Buffers();
void Init_Rendering();

void Render_Scene();

unsigned int Load_Texture(std::string image_path);

void Extend(std::vector<float>& storage, std::vector<float> input);

void BackgroundThread();

void Exit();

void key_proxy(GLFWwindow* window, int key, int scancode, int action, int mods);
void text_proxy(GLFWwindow* window, unsigned int codepoint);
void mouse_proxy(GLFWwindow* window, double posX, double posY);
void scroll_proxy(GLFWwindow* window, double xoffset, double yoffset);
void click_proxy(GLFWwindow* window, int button, int action, int mods);