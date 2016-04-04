#pragma once

#include "classes/Shader.h"
#include "classes/Player.h"
#include "classes/Text.h"

const int SCREEN_WIDTH  = 1920;
const int SCREEN_HEIGHT = 1080;

const int AVG_UPDATE_RANGE = 10;
const int TEXT_UPDATE_FRAME_FREQ = 10;

const int CHUNKS_RENDER_PER_FRAME = 2;

const bool ENABLE_VSYNC = true;

const glm::vec3 CLEAR_COLOR = glm::vec3(0.2f, 0.3f, 0.3f);

double last_fps[AVG_UPDATE_RANGE] = { 0.0 };
double last_cpu[AVG_UPDATE_RANGE] = { 0.0 };

int text_counter = TEXT_UPDATE_FRAME_FREQ;

unsigned int UBO;

double deltaTime = 0.0;
double lastFrame = 0.0;

bool wireframe = false;
bool toggleWireframe = false;

bool EditingChunkQueue = false;
bool EditingDataQueue = false;
bool EditingChunkMap = false;

Player player = Player();

Text* text;
Shader* shader;

GLFWwindow* Window;

std::map<glm::vec3, Chunk*, Vec3Comparator> ChunkMap;
std::map<glm::vec3, std::vector<float>, Vec3Comparator> DataQueue;
std::map<glm::vec3, Chunk*, Vec3Comparator> ChunkQueue;

void Init_GL();
void Init_Textures();
void Init_Text();
void Init_UBO();
void Init_Rendering();

void Update_Data_Queue();

void Render_Scene();
void Draw_UI();

unsigned int Load_Texture(std::string image_path);
std::string Format_Vector(glm::vec3 vector, bool tuple = true, std::string separator = ", ");

void BackgroundThread();

void key_proxy(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_proxy(GLFWwindow* window, double posX, double posY);
void scroll_proxy(GLFWwindow* window, double xoffset, double yoffset);
void click_proxy(GLFWwindow* window, int button, int action, int mods);