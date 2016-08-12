#include "main.h"

// For iterating through directories.
#include <dirent.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

// For reading and writing to files.
#include <sstream>
#include <fstream>

#include "UI.h"
#include "Chat.h"
#include "Timer.h"
#include "Sound.h"
#include "Chunk.h"
#include "Blocks.h"
#include "Camera.h"
#include "Entity.h"
#include "Player.h"
#include "Shader.h"
#include "Worlds.h"
#include "Network.h"
#include "Interface.h"
#include "Inventory.h"

#include "../BlockScripts/Block_Scripts.h"

#ifdef WIN32
	#include <cstdarg>
	#include <Windows.h>

	int __cdecl Print_Debug(const char *format, ...) {
		char str[1024];

		va_list argptr;
		va_start(argptr, format);
		int ret = vsnprintf(str, sizeof(str), format, argptr);
		va_end(argptr);

		OutputDebugStringA(str);

		return ret;
	}
#endif

// The camera drawing limits.
const float Z_NEAR_LIMIT = 0.04f;
const float Z_FAR_LIMIT = 1000.0f;

// Setting default values for variables.
std::string WORLD_NAME = "";
int WORLD_SEED = 0;

std::string PLAYER_NAME = "Player";

double DeltaTime = 0.0;
double LastFrame = 0.0;

bool Wireframe = false;
bool GamePaused = true;
bool Multiplayer = false;
bool MouseEnabled = false;
bool ToggleWireframe = false;

std::atomic_flag ChunkMapBusy = ATOMIC_FLAG_INIT;

static bool WindowFocused = true;
static bool TakeScreenshot = false;
static bool WindowMinimized = false;

static double LastNetworkPositionUpdate = 0.0;

// Initializing objects.
Camera Cam = Camera();
Player player = Player();
Listener listener = Listener();

// Defining buffers.
static UniformBuffer UBO;
static Buffer OutlineBuffer;

// The main window which everything is rendered in.
GLFWwindow* Window = nullptr;

// The map where all the chunks are stored.
// Keys are the chunk's 3D-position.
// Values are pointers to the chunks.
std::unordered_map<glm::vec3, Chunk*, VectorHasher> ChunkMap;

// Defining options.
int AMBIENT_OCCLUSION = 0;
int RENDER_DISTANCE = 4;
int SCREEN_HEIGHT = 1080;
int SCREEN_WIDTH = 1920;
int FULLSCREEN = 0;
int VSYNC = 1;

// Defining shaders.
Shader* shader = nullptr;
Shader* mobShader = nullptr;
Shader* modelShader = nullptr;
Shader* outlineShader = nullptr;

static Time T("Timer");

// List of option references.
static std::map<std::string, int*> Options = {
    {"AmbientOcclusion", &AMBIENT_OCCLUSION},
    {"RenderDistance", &RENDER_DISTANCE},
    {"WindowResY", &SCREEN_HEIGHT},
    {"WindowResX", &SCREEN_WIDTH},
    {"FullScreen", &FULLSCREEN},
    {"VSync", &VSYNC}
};

// Sets settings according to the config file.
void Parse_Config();

// Initialize different objects and states.
void Init_GL();
void Init_Shaders();
void Init_Outline();
void Init_Textures();
void Init_Rendering();

// Renders the main scene.
void Render_Scene();

// The background thread that handles chunk generation.
void Background_Thread();

// Proxy functions that send events to other functions.
void Text_Proxy(GLFWwindow* window, unsigned int codepoint);
void Mouse_Proxy(GLFWwindow* window, double posX, double posY);
void Scroll_Proxy(GLFWwindow* window, double xoffset, double yoffset);
void Click_Proxy(GLFWwindow* window, int button, int action, int mods);
void Key_Proxy(GLFWwindow* window, int key, int scancode, int action, int mods);

void Window_Focused(GLFWwindow* window, int focused);
void Window_Minimized(GLFWwindow* window, int iconified);

int main() {
    // Initialize GLFW, the library responsible for windowing, events, etc...
    glfwInit();

    Parse_Config();
    Blocks::Init();
    Chunks::Load_Structures();

    Init_GL();
    Init_Textures();
    Init_Shaders();
    Init_Outline();
    Init_Rendering();

    UI::Init();
    player.Init();

    Init_Block_Scripts();

    // Start the background thread.
    std::thread chunkGeneration(Background_Thread);

    Network::Init();

    // The main loop.
    // Runs until window is closed.
    while (!glfwWindowShouldClose(Window)) {
        if (WindowMinimized || !WindowFocused) {
            glfwWaitEvents();
			continue;
        }

        // Clear the screen buffer from the last frame.
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Get the time difference between this frame and the last one.
        double currentFrame = glfwGetTime();
        DeltaTime = currentFrame - LastFrame;
        LastFrame = currentFrame;

        // Polls and processes received events.
        glfwPollEvents();

        if (Multiplayer) {
            if (currentFrame - LastNetworkPositionUpdate >= 0.1) {
                LastNetworkPositionUpdate = currentFrame;
                Network::Send_Player_Position();
            }

            Network::Update();
            Network::Update_Players();
            Network::Render_Players();
        }

        if (!GamePaused) {
            // Check if any sounds should be removed.
            listener.Poll_Sounds();

            if (!MouseEnabled && !Chat::Focused) {
                player.Update();
                Entity::Update();
            }

            Render_Scene();
            Entity::Draw();
            player.Draw();
        }

        UI::Draw();

        if (TakeScreenshot) {
            TakeScreenshot = false;
            Take_Screenshot();
        }

        // Swap the newly rendered frame with the old one.
        glfwSwapBuffers(Window);
    }

    if (WORLD_NAME != "") {
        Worlds::Save_World();
    }

    if (Multiplayer) {
        Network::Disconnect();
        Network::Update(1000);
    }

    // On shutting down, join the chunk generation thread with the main thread.
    chunkGeneration.join();

    T.Get("all");

    // Shut down the graphics library, and return.
    glfwTerminate();
    return 0;
}

void Parse_Config() {
    std::stringstream file_content;

    // Load the config file, and store it in file_content.
    std::ifstream file(CONFIG_FILE);

    if (!file.good()) {
        file.close();
        Write_Config();
        return;
    }

    file_content << file.rdbuf();
    file.close();

    std::string line;

    while (std::getline(file_content, line)) {
        // Get the position of the key-value divisor.
        size_t equalPos = line.find('=');
        std::string key = line.substr(0, equalPos);

        if (key != "") {
            *Options[key] = std::stoi(line.substr(equalPos + 1));
        }
    }
}

void Write_Config() {
    // Open the config file for writing.
    std::ofstream file(CONFIG_FILE);

    // For each option, write it and its value to the file.
    for (auto const &option : Options) {
        file << option.first << "=" << *option.second << "\n";
    }

    file.close();
}

void Init_GL() {
    // Set the OpenGL version.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_RESIZABLE, false);
	glfwWindowHint(GLFW_AUTO_ICONIFY, false);

    if (FULLSCREEN) {
        // Stops the window from having any decoration, such as a title bar.
        glfwWindowHint(GLFW_DECORATED, false);

        GLFWmonitor* monitor = glfwGetPrimaryMonitor();

        // Get the video mode of the monitor.
        const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);

        // Set SCREEN_WIDTH and SCREEN_HEIGHT to the resolution of the primary monitor.
        SCREEN_WIDTH = videoMode->width;
        SCREEN_HEIGHT = videoMode->height;

		glfwWindowHint(GLFW_REFRESH_RATE, videoMode->refreshRate);

        // Create a fullscreen window.
        Window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Craftmine", monitor, nullptr);
    }
    else {
        // Set the window to be decorated, allowing users to close it.
        glfwWindowHint(GLFW_DECORATED, true);

        // Create a windowed window.
        Window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Craftmine", nullptr, nullptr);
    }

    // Set the window's position to the upper left corner.
    glfwSetWindowPos(Window, 0, 0);

    // Set the window to be used for future OpenGL calls.
    glfwMakeContextCurrent(Window);

    // Set whether to use VSync based on the value in the config file.
    glfwSwapInterval(VSYNC);

    // Set GLEW to experimental mode (doesn't work otherwise D:)
    glewExperimental = GL_TRUE;

    // Initializes GLEW, which enables vendor-specific OpenGL extensions.
    glewInit();

    // Set all the callback functions for events to the appropiate proxy functions.
    glfwSetKeyCallback(Window, Key_Proxy);
    glfwSetCursorPosCallback(Window, Mouse_Proxy);
    glfwSetScrollCallback(Window, Scroll_Proxy);
    glfwSetMouseButtonCallback(Window, Click_Proxy);
    glfwSetCharCallback(Window, Text_Proxy);
    glfwSetWindowIconifyCallback(Window, Window_Minimized);

    // Enable Blending, which makes transparency work.
    glEnable(GL_BLEND);

    // Enable Fade Culling, which disables rendering of hidden faces.
    glEnable(GL_CULL_FACE);

    // Enable Depth Testing,
    // which chooses which elements to draw based on their depth, thus allowing 3D to work.
    glEnable(GL_DEPTH_TEST);

    // Set the proper function for evaluating blending.
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Specify the size of the OpenGL view port.
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    // Set the color that the window gets filled with when the color buffer gets cleared.
    glClearColor(CLEAR_COLOR.r, CLEAR_COLOR.g, CLEAR_COLOR.b, 1.0f);
}

void Init_Textures() {
    glActiveTexture(GL_TEXTURE0);

    // Load the texture atlas into a texture array, with mipmapping enabled,
    // and store it in the active Texture Unit.
    glBindTexture(GL_TEXTURE_2D_ARRAY, Load_Array_Texture("atlas.png", glm::ivec2(16, 32), 4));
}

void Init_Shaders() {
    // Load the shaders.
    shader        = new Shader("shader");
    mobShader     = new Shader("model2DTex");
    modelShader   = new Shader("model");
    outlineShader = new Shader("outline");

    // Create the frustrum projection matrix for the camera.
    glm::mat4 projection = glm::perspective(
        glm::radians(static_cast<float>(Cam.Zoom)),
        static_cast<float>(SCREEN_WIDTH) / SCREEN_HEIGHT,
        Z_NEAR_LIMIT, Z_FAR_LIMIT
    );

    // Create a matrix storage block in the shaders referenced in the last argument.
    UBO.Create("Matrices", 0, 2 * sizeof(glm::mat4),
        std::vector<Shader*> {shader, outlineShader, modelShader, mobShader}
    );

    UBO.Upload(1, projection);
}

void Init_Outline() {
    // Vector for holding vertex data.
    Data data;

    // The X and Z-values for the vertices.
    int points[4][2] = { {0, 0}, {1, 0}, {1, 1}, {0, 1} };

    // The normal vector components,
    // that helps offset the lines from the surface in order to avoid Z-fighting.
    float n[2] {-1 / sqrtf(3), 1 / sqrtf(3)};

    // Iterate through the Y-values.
    for (int y = 0; y < 3; y++) {
        // Switch from horizontal to vertical lines if y is equal to 2.
        glm::ivec2 yVec = (y == 2) ? glm::ivec2(0, 1) : glm::ivec2(y);

        // Iterate over the vertices.
        for (int i = 0; i < 4; i++) {
            // If y is equal to 2, increment the index by 1, with overflow protection.
            int index = (y == 2) ? i : (i + 1) % 4;

            // Store the data in the vector.
            Extend(data,
                points[i][0], yVec.x, points[i][1],
                n[points[i][0]], n[yVec.x], n[points[i][1]],
                points[index][0], yVec.y, points[index][1],
                n[points[index][0]], n[yVec.y], n[points[index][1]]
            );
        }
    }

    OutlineBuffer.Init(outlineShader);
    OutlineBuffer.Create(3, 3, data);
    OutlineBuffer.VertexType = GL_LINES;
}

void Init_Rendering() {
    // Default identity matrix, does nothing.
    glm::mat4 model;

    // Upload the empty matrix.
    shader->Upload("model", model);

    // Upload the light level of an unlit block.
    shader->Upload("ambient", AMBIENT_LIGHT);

    // Upload the max light level of a block.
    shader->Upload("diffuse", DIFFUSE_LIGHT);

    // Upload the texture unit index of the main textures.
    shader->Upload("diffTex", 0);
    modelShader->Upload("tex", 0);
}

void Render_Scene() {
    UBO.Upload(0, Cam.GetViewMatrix());

    if (ToggleWireframe) {
        Wireframe = !Wireframe;
        ToggleWireframe = false;

        // Toggle rendering mode.
        glPolygonMode(GL_FRONT_AND_BACK, Wireframe ? GL_LINE : GL_FILL);

        // If using wireframe,
        // upload an invalid texture unit to make wireframe lines black.
        shader->Upload("diffTex", Wireframe ? 50 : 0);
    }

    // Set the first rendering pass to discard any transparent fragments.
    shader->Upload("RenderTransparent", false);

    for (auto const &chunk : ChunkMap) {
		chunk.second->Draw();
    }

    // Set the second rendering pass to discard any opaque fragments.
    shader->Upload("RenderTransparent", true);

    for (auto const &chunk : ChunkMap) {
		chunk.second->Draw(true);
    }

    if (!player.LookingAtBlock || player.LookingBlockType == nullptr) {
        return;
    }

    // Start with an empty identity matrix.
    glm::mat4 model;

    // Translate the outline, and scale it to the block size.
    model = glm::translate(
        model, Get_World_Pos(player.LookingChunk, player.LookingTile) + player.LookingBlockType->ScaleOffset
    );
    model = glm::scale(model, player.LookingBlockType->Scale);

    // Upload the matrix, and draw the outline.
    outlineShader->Upload("model", model);
    OutlineBuffer.Draw();
}

void Background_Thread() {
	while (true) {
		if (glfwWindowShouldClose(Window)) {
			return;
		}

		if (GamePaused) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}

		bool queueEmpty = true;

        // Waits for the chunk map to be available for reading.
        while (ChunkMapBusy.test_and_set(std::memory_order_acquire)) {
			;
        }

        // Get the XZ-location of the player.
        glm::vec2 playerPos = player.CurrentChunk.xz();
        float nearestDistance = static_cast<float>(RENDER_DISTANCE);
        Chunk* nearestChunk = nullptr;

        for (auto const &chunk : ChunkMap) {
			// Get the distance between the chunk and the player's position.
			float dist = glm::distance(chunk.first.xz(), playerPos);

			// If the distance is smaller than the smallest so far,
			// set the chunk to be the nearest chunk.
			if (dist >= RENDER_DISTANCE) {
				continue;
			}

			if (chunk.second->Meshed) {
				continue;
			}

            if (dist < nearestDistance) {
                nearestDistance = dist;
                nearestChunk = chunk.second;
            }
			else if (dist == nearestDistance) {
				if (chunk.first.y > nearestChunk->Position.y) {
					nearestChunk = chunk.second;
				}
			}
        }

        // Checks if there's a chunk to be rendered.
        if (nearestChunk != nullptr) {
            if (!nearestChunk->Generated) {
                nearestChunk->Generate();
            }

            nearestChunk->Light();
            nearestChunk->Mesh();

            nearestChunk->Meshed = true;
            nearestChunk->DataUploaded = false;
            queueEmpty = false;
        }

        // Show that the thread is no longer using the chunk map.
		ChunkMapBusy.clear(std::memory_order_release);

        // Sleep for 1 ms if there's still chunks to be generated, else sleep for 100 ms.
        std::this_thread::sleep_for(std::chrono::milliseconds(queueEmpty ? 100 : 1));
    }
}

#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wunused-parameter"
#elif _MSC_VER
    #pragma warning(push)
    #pragma warning(disable: 4100)
#endif

void Key_Proxy(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_L && action == GLFW_PRESS) {
        TakeScreenshot = true;
    }

    if (GamePaused) {
        UI::Key_Handler(key, action);
    }
    else {
        if (Chat::Focused) {
            player.Clear_Keys();
        }
        else {
            UI::Key_Handler(key, action);
            player.Key_Handler(key, action);

            if (Multiplayer && action != GLFW_REPEAT) {
                Network::Send_Key_Event(key, action);
            }
        }

        if (action == GLFW_PRESS) {
            Chat::Key_Handler(key);
        }
    }
}

void Mouse_Proxy(GLFWwindow* window, double posX, double posY) {
    UI::Mouse_Handler(
        static_cast<int>(posX),
        static_cast<int>(posY)
    );
}

// Proxy for receiving Unicode codepoints, very useful for getting text input.
void Text_Proxy(GLFWwindow* window, unsigned int codepoint) { UI::Text_Handler(codepoint); }

void Scroll_Proxy(GLFWwindow* window, double offsetX, double offsetY) { if (!GamePaused) { player.Scroll_Handler(offsetY); } }
void Click_Proxy(GLFWwindow* window, int button, int action, int mods) { UI::Click(action, button); }

void Window_Focused(GLFWwindow* window, int focused) { WindowFocused = focused > 0; }
void Window_Minimized(GLFWwindow* window, int iconified) { WindowMinimized = iconified > 0; }

void Exit(void* caller) { glfwSetWindowShouldClose(Window, true); }

#ifdef __clang__
    #pragma clang diagnostic pop
#elif _MSC_VER
    #pragma warning(pop)
#endif
