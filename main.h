#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>

#define GLM_SWIZZLE
#include <glm/glm.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#ifdef _WIN32
const bool Windows = true;

#elif __APPLE__
const bool Windows = false;

#endif

// TODO: Interpolate lighting by having different values for vertices per block.

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
    { {0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {1, 1, 1}, {0, 1, 1}, {0, 0, 1} }
};

glm::vec2 tex_coords[6][6] = {
    { {0, 1}, {1, 0}, {0, 0}, {1, 0}, {0, 1}, {1, 1} },
    { {1, 1}, {1, 0}, {0, 0}, {1, 1}, {0, 0}, {0, 1} },
    { {0, 0}, {1, 0}, {1, 1}, {1, 1}, {0, 1}, {0, 0} },
    { {1, 0}, {0, 1}, {1, 1}, {1, 0}, {0, 0}, {0, 1} },
    { {1, 1}, {0, 0}, {0, 1}, {1, 1}, {1, 0}, {0, 0} },
    { {0, 1}, {1, 1}, {1, 0}, {1, 0}, {0, 0}, {0, 1} }
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
int AmbientOcclusion;

std::map<std::string, int*> Options = {
    {"RenderDistance", &RenderDistance},
    {"FullScreen", &Fullscreen},
    {"WindowResX", &SCREEN_WIDTH},
    {"WindowResY", &SCREEN_HEIGHT},
    {"VSync", &VSync},
    {"AmbientOcclusion", &AmbientOcclusion}
};

glm::vec2 IMAGE_SIZE = glm::vec2(16, 32);

double DeltaTime = 0.0;
double LastFrame = 0.0;

bool Wireframe = false;
bool ToggleWireframe = false;

bool GamePaused = true;
bool MouseEnabled = false;

bool ChunkMapBusy = false;