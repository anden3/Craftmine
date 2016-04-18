#pragma once

#include "Shader.h"
#include "UI.h"

#ifdef _WIN32
const bool Windows = true;

#elif __APPLE__
const bool Windows = false;

#endif

const int FONT_SIZE = 15;
const glm::vec3 CLEAR_COLOR = glm::vec3(0.2f, 0.3f, 0.3f);

int SCREEN_WIDTH = 1920;
int SCREEN_HEIGHT = 1080;
int RENDER_DISTANCE = 5;

bool VSync = true;

unsigned int DebugVBO, DebugVAO;
unsigned int OutlineVBO, OutlineVAO;
unsigned int UBO;

double DeltaTime = 0.0;
double LastFrame = 0.0;

bool Wireframe = false;
bool ToggleWireframe = false;

bool EditingChunkQueue = false;
bool EditingDataQueue = false;
bool EditingChunkMap = false;

Player player = Player();
Chat chat = Chat();

Shader* shader;
Shader* outlineShader;

int modelMatrixLocation;
int diffuseTextureLocation;

GLFWwindow* Window;

std::map<glm::vec3, Chunk*, Vec3Comparator> ChunkMap;
std::map<glm::vec3, std::vector<float>, Vec3Comparator> DataQueue;
std::map<glm::vec3, Chunk*, Vec3Comparator> ChunkQueue;

void Init_GL();
void Init_Textures();
void Init_Shaders();
void Init_Buffers();
void Init_Rendering();

void Update_Data_Queue();
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