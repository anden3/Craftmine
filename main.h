#pragma once

#include <iostream>

#include "classes/Shader.h"
#include "classes/Player.h"

struct Character {
    unsigned int TextureID;
    glm::ivec2 Size;
    glm::ivec2 Bearing;
    unsigned int Advance;
};

const int SCREEN_WIDTH  = 1440;
const int SCREEN_HEIGHT = 900;

const int AVG_FPS_RANGE = 10;
const int TEXT_UPDATE_FRAME_FREQ = 10;

const int TEXT_TEXTURE_UNIT = 10;

const glm::vec3 TEXT_COLOR = glm::vec3(0.2f, 0.8f, 0.2f);

double last_fps[AVG_FPS_RANGE] = {0.0};

std::string current_RAM;
std::string current_FPS;

int text_counter = TEXT_UPDATE_FRAME_FREQ;

unsigned int UBO;
unsigned int textVAO, textVBO;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool wireframe = false;
bool toggleWireframe = false;

Player player = Player();

std::map<char, Character> Characters;
std::map<glm::vec3, Chunk*, Vec3Comparator> ChunkMap;
std::vector<Chunk*> ChunkQueue;

void Generate_Chunk();

void Draw_UI(Shader shader, float deltaTime);
void Render_Scene(Shader shader);

void Init_Text(Shader shader);
void Render_Text(Shader shader, std::string text, float x, float y, float scale, glm::vec3 color);

unsigned int loadTexture(std::string image);
std::string getMemoryUsage();
std::string formatVector(glm::vec3 vector, bool tuple = true, std::string separator = ", ");

void key_proxy(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_proxy(GLFWwindow* window, double posX, double posY);
void scroll_proxy(GLFWwindow* window, double xoffset, double yoffset);
void click_proxy(GLFWwindow* window, int button, int action, int mods);