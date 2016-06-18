#pragma once

#define GLM_SWIZZLE
#include <glm/glm.hpp>

#include <map>
#include <string>
#include <vector>

// Check if program is running on Windows or OS X.
#ifdef _WIN32
const bool Windows = true;
#elif __APPLE__
const bool Windows = false;
#endif

// Compares two vec3 objects, and returns the smallest one.
class VectorComparator {
 public:
    bool operator () (const glm::vec3 &a, const glm::vec3 &b) const {
        if (a.x != b.x) {
            return a.x < b.x;
        }
        else if (a.z != b.z) {
            return a.z < b.z;
        }
        else if (a.y != b.y) {
            return a.y > b.y;
        }
        return false;
    }
};

// Forward declaring classes.
class Chat;
class Chunk;
class Camera;
class Player;
class Shader;
class Listener;
class Interface;
class Inventory;
struct GLFWwindow;
class NetworkClient;

// Declaring shaders.
extern Shader* shader;
extern Shader* modelShader;
extern Shader* outlineShader;
extern Shader* mobShader;

// Defining references to objects.
extern Chat chat;
extern Camera Cam;
extern Player player;
extern Listener listener;
extern Interface interface;
extern Inventory inventory;
extern GLFWwindow* Window;
extern NetworkClient Client;

extern std::map<glm::vec3, Chunk*, VectorComparator> ChunkMap;

// TODO-list.
// TODO: Interpolate lighting by having different values for vertices per block.

// The color that the screen gets filled with when the color buffer is cleared.
const glm::vec3 CLEAR_COLOR = glm::vec3(0.529f, 0.808f, 0.922f);

// The light level of unlit blocks.
const glm::vec3 AMBIENT_LIGHT = glm::vec3(0.1f);

// The maximum light level of lit blocks.
const glm::vec3 DIFFUSE_LIGHT = glm::vec3(0.7f);

// The light level of blocks in direct sunlight.
const int SUN_LIGHT_LEVEL = 15;

// The number that all noise coordinates are divided by.
// Higher values equals more zoom.
const int CHUNK_ZOOM = 50;

// The minimum noise density required to be a solid block.
const double NOISE_DENSITY_BLOCK = 0.5;

// The minimum noise density required to be a solid block during cave generation.
const double NOISE_DENSITY_CAVE = -0.85;

// The file to load settings from.
const char CONFIG_FILE[] = "config.conf";

const char PLAYER_NAME[] = "anden3";

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
    {"head", { {{16,  8}, {24, 16}}, {{0,   8}, {8,  16}}, {{16,  0}, {24,  8}}, {{8,   0}, {16,  8}}, {{8,   8}, {16, 16}}, {{24,  8}, {32, 16}} }},
    {"body", { {{28, 20}, {32, 32}}, {{16, 20}, {20, 32}}, {{28, 16}, {36, 20}}, {{20, 16}, {28, 20}}, {{20, 20}, {28, 32}}, {{32, 20}, {40, 32}} }},
    {"leg",  { {{8,  20}, {12, 32}}, {{0,  20}, {4,  32}}, {{8,  16}, {12, 20}}, {{4,  16}, {8,  20}}, {{4,  20}, {8,  32}}, {{12, 20}, {16, 32}} }},
    {"arm",  { {{48, 20}, {52, 32}}, {{40, 20}, {40, 32}}, {{48, 16}, {52, 20}}, {{44, 16}, {48, 20}}, {{44, 20}, {48, 32}}, {{52, 20}, {56, 32}} }},
};

// Order of directions for block faces.
enum Directions {LEFT, RIGHT, DOWN, UP, BACK, FRONT};

// The resolution of the window.
// If fullscreen is off, it's set to the values in the config file.
// Else, it's set to the resolution of the primary monitor.
extern int SCREEN_WIDTH, SCREEN_HEIGHT;

// If window is in fullscreen mode.
extern int FULLSCREEN;

// If VSync is enabled.
extern int VSYNC;

// The radius (in chunks) around the player to render.
extern int RENDER_DISTANCE;

// If ambient occlusion is enabled.
extern int AMBIENT_OCCLUSION;

// Used for storing the time difference between the current frame and the last (in seconds).
extern double DeltaTime;

// Used for storing the timestamp (in seconds) of the last frame.
extern double LastFrame;

// If wireframe is enabled.
extern bool Wireframe;

// If set to true, will toggle the wireframe state.
extern bool ToggleWireframe;

// If the game is paused.
extern bool GamePaused;

// If the mouse cursor is visible.
extern bool MouseEnabled;

// If the chunk map is currently locked to a thread.
extern bool ChunkMapBusy;

void Write_Config();
void Exit();
