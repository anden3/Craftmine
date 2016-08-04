#pragma once

#include <map>
#include <atomic>
#include <string>
#include <vector>
#include <unordered_map>

#include "Comparators.h"

// Check if program is running on Windows or OS X.
#ifdef _WIN32
    #undef interface
    const bool Windows = true;

	int __cdecl Print_Debug(const char *format, ...);
#elif __APPLE__
    const bool Windows = false;
#endif

// Forward declaring classes.
class Chunk;
class Camera;
class Player;
class Shader;
class Listener;
struct GLFWwindow;

// Declaring shaders.
extern Shader* shader;
extern Shader* mobShader;
extern Shader* modelShader;
extern Shader* outlineShader;

// Defining references to objects.
extern Camera Cam;
extern Player player;
extern Listener listener;
extern GLFWwindow* Window;

extern std::unordered_map<glm::vec3, Chunk*, VectorHasher> ChunkMap;

extern std::string WORLD_NAME;
extern int WORLD_SEED;

extern std::string PLAYER_NAME;

// The color that the screen gets filled with when the color buffer is cleared.
const glm::vec3 CLEAR_COLOR = glm::vec3(0.529f, 0.808f, 0.922f);

// The light level of unlit blocks.
const glm::vec3 AMBIENT_LIGHT = glm::vec3(0.1f);

// The maximum light level of lit blocks.
const glm::vec3 DIFFUSE_LIGHT = glm::vec3(0.7f);

// The light level of blocks in direct sunlight.
const int SUN_LIGHT_LEVEL = 15;

const int PLAYER_TEXTURE_UNIT = 5;

// The number that all noise coordinates are divided by.
// Higher values equals more zoom.
const int CHUNK_ZOOM = 50;

// The minimum noise density required to be a solid block.
const double NOISE_DENSITY_BLOCK = 0.5;

// The minimum noise density required to be a solid block during cave generation.
const double NOISE_DENSITY_CAVE = -0.85;

const float GRAVITY = 0.004f;

const float PLAYER_BASE_SPEED      = 3.0f;
const float PLAYER_SPRINT_MODIFIER = 1.5f;
const float PLAYER_WIDTH           = 0.1f;

const float CAMERA_HEIGHT          = 1.7f;
const float JUMP_HEIGHT            = 0.1f;

// The file to load settings from.
const char CONFIG_FILE[] = "config.conf";

// Vertex coordinates for a 3D-block.
// Side order is: Left - Right - Down - Up - Back - Front.
const glm::vec3 vertices[6][6] = {
    { {0, 0, 0}, {0, 1, 1}, {0, 1, 0}, {0, 1, 1}, {0, 0, 0}, {0, 0, 1} },
    { {1, 0, 0}, {1, 1, 0}, {1, 1, 1}, {1, 0, 0}, {1, 1, 1}, {1, 0, 1} },
    { {0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {1, 0, 1}, {0, 0, 1}, {0, 0, 0} },
    { {0, 1, 0}, {1, 1, 1}, {1, 1, 0}, {0, 1, 0}, {0, 1, 1}, {1, 1, 1} },
    { {0, 0, 0}, {1, 1, 0}, {1, 0, 0}, {0, 0, 0}, {0, 1, 0}, {1, 1, 0} },
    { {0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {1, 1, 1}, {0, 1, 1}, {0, 0, 1} }
};

// Texture coordinates for a 3D-block.
// Same side order as above.
const glm::vec2 tex_coords[6][6] = {
    { {0, 1}, {1, 0}, {0, 0}, {1, 0}, {0, 1}, {1, 1} },
    { {1, 1}, {1, 0}, {0, 0}, {1, 1}, {0, 0}, {0, 1} },
    { {0, 0}, {1, 0}, {1, 1}, {1, 1}, {0, 1}, {0, 0} },
    { {1, 0}, {0, 1}, {1, 1}, {1, 0}, {0, 0}, {0, 1} },
    { {1, 1}, {0, 0}, {0, 1}, {1, 1}, {1, 0}, {0, 0} },
    { {0, 1}, {1, 1}, {1, 0}, {1, 0}, {0, 0}, {0, 1} }
};

// Texture coordinates for the different body parts of the player model.
const std::map<std::string, std::vector<std::vector<glm::vec2>>> PlayerTexCoords = {
    {"head", {
        {{16,  8}, {24, 16}}, {{0,   8}, {8,  16}}, {{16,  0}, {24,  8}},
        {{8,   0}, {16,  8}}, {{8,   8}, {16, 16}}, {{24,  8}, {32, 16}}
    }},
    {"body", {
        {{28, 20}, {32, 32}}, {{16, 20}, {20, 32}}, {{28, 16}, {36, 20}},
        {{20, 16}, {28, 20}}, {{20, 20}, {28, 32}}, {{32, 20}, {40, 32}}
    }},
    {"leg",  {
        {{8,  20}, {12, 32}}, {{0,  20}, {4,  32}}, {{8,  16}, {12, 20}},
        {{4,  16}, {8,  20}}, {{4,  20}, {8,  32}}, {{12, 20}, {16, 32}}
    }},
    {"arm",  {
        {{48, 20}, {52, 32}}, {{40, 20}, {40, 32}}, {{48, 16}, {52, 20}},
        {{44, 16}, {48, 20}}, {{44, 20}, {48, 32}}, {{52, 20}, {56, 32}}
    }}
};

// Order of directions for block faces.
enum Directions {LEFT, RIGHT, DOWN, UP, BACK, FRONT};

// The resolution of the window.
// If fullscreen is off, it's set to the values in the config file.
// Else, it's set to the resolution of the primary monitor.
extern int SCREEN_WIDTH, SCREEN_HEIGHT;

extern int VSYNC;
extern int FULLSCREEN;
extern int AMBIENT_OCCLUSION;

// The radius (in chunks) around the player to render.
extern int RENDER_DISTANCE;

// Used for storing the time difference between the current frame and the last (in seconds).
extern double DeltaTime;

// Used for storing the timestamp (in seconds) of the last frame.
extern double LastFrame;

extern bool Wireframe;
extern bool GamePaused;
extern bool ToggleWireframe;

// If the mouse cursor is visible.
extern bool MouseEnabled;

// If the chunk map is currently locked to a thread.
extern std::atomic_flag ChunkMapBusy;

extern bool Multiplayer;

void Write_Config();
void Exit(void* caller);
