#pragma once

#include "Shader.h"
#include "UI.h"

#ifdef _WIN32
const bool Windows = true;

#elif __APPLE__
const bool Windows = false;

#endif

const glm::vec3 CLEAR_COLOR = glm::vec3(0.2f, 0.3f, 0.3f);
const std::string CONFIG_FILE = "config.conf";

typedef std::vector<glm::vec2> TexArray;
typedef glm::vec2 Coord;

std::map<unsigned int, glm::vec2> textureCoords = {
    
    // BLOCKS
    {1,   Coord( 2,  1)}, // Stone
    {2,   Coord( 1,  1)}, // Grass Top
    {3,   Coord( 3,  1)}, // Dirt
    {4,   Coord( 1,  2)}, // Cobblestone
    {5,   Coord( 5,  1)}, // Wooden Planks
    {6,   Coord(16,  1)}, // Oak Sapling
    {7,   Coord( 2,  2)}, // Bedrock
    {8,   Coord(13, 14)}, // Flowing Water
    {9,   Coord(13, 14)}, // Still Water
    {10,  Coord(15, 14)}, // Flowing Lava
    {11,  Coord(15, 14)}, // Still Lava
    {12,  Coord( 2,  3)}, // Sand
    {13,  Coord( 2,  4)}, // Gravel
    {14,  Coord( 3,  1)}, // Gold Ore
    {15,  Coord( 3,  2)}, // Iron Ore
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
    
    {65,  Coord( 4,  6)}, // Ladder
    {66,  Coord( 1,  9)}, // Rail
    
    {69,  Coord( 1,  7)}, // Lever
    
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
    {256, Coord(3 , 22)}, // Iron Shovel
    {257, Coord(3 , 23)}, // Iron Pickaxe
    {258, Coord(3 , 24)}, // Iron Axe
    {259, Coord(6 , 17)}, // Flint and Steel
    {260, Coord(11, 17)}, // Apple
    {261, Coord(6 , 18)}, // Bow
    {262, Coord(6 , 19)}, // Arrow
    {263, Coord(8 , 17)}, // Coal
    {264, Coord(8 , 20)}, // Diamond
    {265, Coord(8 , 18)}, // Iron Ingot
    {266, Coord(8 , 19)}, // Gold Ingot
    
    {280, Coord(6 , 20)}, // Stick
};

std::map<unsigned int, TexArray> MultiTextures = {
    //             LEFT           RIGHT          DOWN           UP             BACK           FRONT
    {2,  TexArray {Coord( 4,  1), Coord( 4,  1), Coord( 3,  1), Coord( 1,  1), Coord( 4,  1), Coord( 4,  1)}}, // Grass
    {17, TexArray {Coord( 5,  2), Coord( 5,  2), Coord( 6,  2), Coord( 6,  2), Coord( 5,  2), Coord( 5,  2)}}, // Oak Log
    {23, TexArray {Coord(14,  3), Coord(14,  3), Coord(15,  4), Coord(15,  4), Coord(14,  3), Coord(15,  3)}}, // Dispenser
    {24, TexArray {Coord( 1, 13), Coord( 1, 13), Coord( 1, 12), Coord( 1, 12), Coord( 1, 13), Coord( 1, 13)}}, // Sandstone
    {25, TexArray {Coord(11,  5), Coord(11,  5), Coord(11,  5), Coord(12,  5), Coord(11,  5), Coord(11,  5)}}, // Note Block
    {46, TexArray {Coord( 9,  1), Coord( 9,  1), Coord(11,  1), Coord(10,  1), Coord( 9,  1), Coord( 9,  1)}}, // TNT
    {47, TexArray {Coord( 4,  3), Coord( 4,  3), Coord(5 ,  1), Coord(5 ,  1), Coord( 4,  3), Coord( 4,  3)}}, // Bookshelf
    {54, TexArray {Coord(11,  2), Coord(11,  2), Coord(10,  2), Coord(10,  2), Coord(11,  2), Coord(12,  2)}}, // Chest
    {58, TexArray {Coord(12,  4), Coord(12,  4), Coord( 5,  1), Coord(12,  3), Coord(13,  4), Coord(13,  4)}}, // Crafting Table
    {60, TexArray {Coord( 3,  1), Coord( 3,  1), Coord( 3,  1), Coord( 8,  6), Coord( 3,  1), Coord( 3,  1)}}, // Farmland
    {61, TexArray {Coord(14,  3), Coord(14,  3), Coord(15,  4), Coord(15,  4), Coord(14,  3), Coord(13,  3)}}, // Furnace
    {62, TexArray {Coord(14,  3), Coord(14,  3), Coord(15,  4), Coord(15,  4), Coord(14,  3), Coord(14,  4)}}, // Burning Furnace
};

std::map<unsigned int, float> blockHardness = {
    {1, 1.5f},
    {2, 0.5f},
    {3, 0.5f}
};

std::map<std::string, std::vector<unsigned int>> BlockSounds = {
    {"cloth", std::vector<unsigned int> {}},
    {"dirt", std::vector<unsigned int> {2, 3}},
    {"gravel", std::vector<unsigned int> {13}},
    {"sand", std::vector<unsigned int> {12}},
    {"snow", std::vector<unsigned int> {}},
    {"stone", std::vector<unsigned int> {1, 4, 7, 14, 15, 16}},
    {"wood", std::vector<unsigned int> {5}}
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