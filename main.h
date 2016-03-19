#pragma once

#include <map>
#include <iostream>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "classes/Shader.h"
#include "classes/Player.h"

struct Character {
    unsigned int TextureID;
    glm::ivec2 Size;
    glm::ivec2 Bearing;
    unsigned int Advance;
};

static const int SCREEN_WIDTH  = 1440;
static const int SCREEN_HEIGHT = 900;

static const int AVG_FPS_RANGE = 5;
static const int FPS_UPDATE_FRAME_FREQ = 5;

static const int TEXT_TEXTURE_UNIT = 10;

static double last_fps[AVG_FPS_RANGE] = {0.0};
static int fps_counter = 0;
static int current_FPS = 0;

unsigned int UBO;
unsigned int textVAO, textVBO;

float lastX = SCREEN_WIDTH / 2, lastY = SCREEN_HEIGHT / 2;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool keys[1024];
bool firstMouse = true;

Player player = Player();

std::map<char, Character> Characters;
std::map<glm::vec3, Chunk*, Vec3Comparator> ChunkMap;
std::vector<Chunk*> ChunkQueue;

void Generate_Chunk();
void Draw_Text(Shader shader, float deltaTime);
void Render_Scene(Shader shader);
void Do_Movement(float deltaTime);
void Init_Text(Shader shader);
void Render_Text(Shader shader, std::string text, float x, float y, float scale, glm::vec3 color);
unsigned int loadTexture(std::string image);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);