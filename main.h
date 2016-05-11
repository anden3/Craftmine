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

std::map<unsigned char, glm::vec2> textureCoords = {
    {1,  glm::vec2(  2,   1)}, // Stone
    
    {2,  glm::vec2(  1,   1)}, // Grass Top
    {3,  glm::vec2(  3,   1)}, // Dirt
    {4,  glm::vec2(  1,   2)}, // Cobblestone
    {5,  glm::vec2(  5,   1)}, // Wooden Planks
    
    {7,  glm::vec2(  2,   2)}, // Bedrock
    
    {9,  glm::vec2( 13,  14)}, // Water
    {11, glm::vec2( 15,  14)}, // Lava
    
    {12, glm::vec2(  2,   3)}, // Sand
    {13, glm::vec2(  2,   4)}, // Gravel
    
    {14, glm::vec2(  3,   1)}, // Gold Ore
    {15, glm::vec2(  3,   2)}, // Iron Ore
    {16, glm::vec2(  3,   3)}, // Coal Ore
    
    {17, glm::vec2(  5,   2)}, // Oak Log
    
    {18, glm::vec2(  5,   4)}, // Transparent Leaves
    
    {50, glm::vec2(  1,   6)}, // Torch
    
    // Damage Textures
    {246, glm::vec2( 1,  16)},
    {247, glm::vec2( 2,  16)},
    {248, glm::vec2( 3,  16)},
    {249, glm::vec2( 4,  16)},
    {250, glm::vec2( 5,  16)},
    {251, glm::vec2( 6,  16)},
    {252, glm::vec2( 7,  16)},
    {253, glm::vec2( 8,  16)},
    {254, glm::vec2( 9,  16)},
    {255, glm::vec2(10,  16)}
};

std::map<unsigned char, float> blockHardness = {
    {1, 1.5f},
    {2, 0.5f},
    {3, 0.5f}
};

std::map<std::string, std::vector<unsigned char>> BlockSounds = {
    {"cloth", std::vector<unsigned char> {}},
    {"dirt", std::vector<unsigned char> {2, 3}},
    {"gravel", std::vector<unsigned char> {13}},
    {"sand", std::vector<unsigned char> {12}},
    {"snow", std::vector<unsigned char> {}},
    {"stone", std::vector<unsigned char> {1, 4, 7, 14, 15, 16}},
    {"wood", std::vector<unsigned char> {5}}
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