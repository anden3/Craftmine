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
#include "Network.h"
#include "Interface.h"
#include "Inventory.h"

// The camera drawing limits.
const float Z_NEAR_LIMIT = 0.04f;
const float Z_FAR_LIMIT = 1000.0f;

// Setting default values for variables.
double DeltaTime = 0.0;
double LastFrame = 0.0;

bool Wireframe = false;
bool GamePaused = true;
bool Multiplayer = false;
bool MouseEnabled = false;
bool ChunkMapBusy = false;
bool ToggleWireframe = false;

// Initializing objects.
Chat chat = Chat();
Camera Cam = Camera();
Player player = Player();
Listener listener = Listener();
Interface interface = Interface();
Inventory inventory = Inventory();
NetworkClient Client = NetworkClient();

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
int RENDER_DISTANCE = 0;
int FULLSCREEN = 0;
int SCREEN_WIDTH = 1920;
int SCREEN_HEIGHT = 1080;
int VSYNC = 0;
int AMBIENT_OCCLUSION = 0;

// Defining shaders.
Shader* shader = nullptr;
Shader* modelShader = nullptr;
Shader* outlineShader = nullptr;
Shader* mobShader = nullptr;

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
void Init_Textures();
void Init_Shaders();
void Init_Outline();
void Init_Rendering();

// Renders the main scene.
void Render_Scene();

// The background thread that handles chunk generation.
void Background_Thread();

// Proxy functions that send events to other functions.
void key_proxy(GLFWwindow* window, int key, int scancode, int action, int mods);
void text_proxy(GLFWwindow* window, unsigned int codepoint);
void mouse_proxy(GLFWwindow* window, double posX, double posY);
void scroll_proxy(GLFWwindow* window, double xoffset, double yoffset);
void click_proxy(GLFWwindow* window, int button, int action, int mods);

int main() {
    // Initialize GLFW, the library responsible for windowing, events, etc...
    glfwInit();

    Parse_Config();
    Blocks::Init();

    Init_GL();
    Init_Textures();
    Init_Shaders();
    Init_Outline();
    Init_Rendering();

    UI::Init();
    player.Init();

    // Start the background thread.
    std::thread chunkGeneration(Background_Thread);

    player.Queue_Chunks();
    Client.Init(PLAYER_NAME);

    if (Multiplayer) {
        Client.Connect("localhost", 1234);
    }

    // The main loop.
    // Runs until window is closed.
    while (!glfwWindowShouldClose(Window)) {
        // Clear the screen buffer from the last frame.
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Get the time difference between this frame and the last one.
        double currentFrame = glfwGetTime();
        DeltaTime = currentFrame - LastFrame;
        LastFrame = currentFrame;

        // Polls and processes received events.
        glfwPollEvents();

        if (Multiplayer) {
            Client.Update();
        }

        if (!GamePaused) {
            // Check if any sounds should be removed.
            listener.Poll_Sounds();

            if (!MouseEnabled && !chat.Focused) {
                player.Update();
                Entity::Update();
            }

            Render_Scene();
            Entity::Draw();
            player.Draw();
        }

        UI::Draw();

        if (keys[GLFW_KEY_L]) {
            Take_Screenshot();
        }

        // Swap the newly rendered frame with the old one.
        glfwSwapBuffers(Window);
    }

    if (Multiplayer) {
        Client.Disconnect();
        Client.Update(1000);
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
    file_content << file.rdbuf();
    file.close();

    std::string line;

    while (std::getline(file_content, line)) {
        // Get the position of the key-value divisor.
        unsigned long equalPos = line.find('=');
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

    if (FULLSCREEN) {
        // Stops the window from having any decoration, such as a title bar.
        glfwWindowHint(GLFW_DECORATED, false);

        GLFWmonitor* monitor = glfwGetPrimaryMonitor();

        // Get the video mode of the monitor.
        const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);

        // Set SCREEN_WIDTH and SCREEN_HEIGHT to the resolution of the primary monitor.
        SCREEN_WIDTH = videoMode->width;
        SCREEN_HEIGHT = videoMode->height;

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
    glfwSetKeyCallback(Window, key_proxy);
    glfwSetCursorPosCallback(Window, mouse_proxy);
    glfwSetScrollCallback(Window, scroll_proxy);
    glfwSetMouseButtonCallback(Window, click_proxy);
    glfwSetCharCallback(Window, text_proxy);

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
    shader = new Shader("shader");
    modelShader = new Shader("model");
    mobShader = new Shader("model2DTex");
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

    if (!player.LookingAtBlock) {
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

        bool queueEmpty = true;

        // Waits for the chunk map to be available for reading.
        while (ChunkMapBusy) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        // Stops the chunk map from being modified.
        ChunkMapBusy = true;

        // Get the XZ-location of the player.
        glm::vec2 playerPos = player.CurrentChunk.xz();
        float nearestDistance = RENDER_DISTANCE;
        Chunk* nearestChunk = nullptr;

        for (auto const &chunk : ChunkMap) {
            if (!chunk.second->Meshed) {
                // Get the distance between the chunk and the player's position.
                float dist = glm::distance(chunk.first.xz(), playerPos);

                // If the distance is smaller than the smallest so far,
                // set the chunk to be the nearest chunk.
                if (dist < nearestDistance && dist < RENDER_DISTANCE) {
                    nearestDistance = dist;
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
        ChunkMapBusy = false;

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

void key_proxy(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (GamePaused) {
        UI::Key_Handler(key, action);
    }
    else {
        if (chat.Focused) {
            player.Clear_Keys();
        }
        else {
            UI::Key_Handler(key, action);
            player.Key_Handler(key, action);
        }

        if (action == GLFW_PRESS) {
            chat.Key_Handler(key);
        }
    }
}
// Proxy for receiving Unicode codepoints, very useful for getting text input.
void text_proxy(GLFWwindow* window, unsigned int codepoint) {
    if (chat.Focused && !chat.FocusToggled) {
        chat.Input(codepoint);
    }
}
void mouse_proxy(GLFWwindow* window, double posX, double posY) {
    UI::Mouse_Handler(posX, posY);

    if (!GamePaused && !chat.Focused) {
        player.Mouse_Handler(posX, posY);
    }
}
void scroll_proxy(GLFWwindow* window, double offsetX, double offsetY) {
    if (!GamePaused) {
        player.Scroll_Handler(offsetY);
    }
}
void click_proxy(GLFWwindow* window, int button, int action, int mods) {
    UI::Click(action, button);

    if (!GamePaused) {
        player.Click_Handler(button, action);
    }
}

#ifdef __clang__
    #pragma clang diagnostic pop
#elif _MSC_VER
    #pragma warning(pop)
#endif

void Exit() {
    glfwSetWindowShouldClose(Window, GL_TRUE);
}
