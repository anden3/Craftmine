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
#include "Sound.h"
#include "Chunk.h"
#include "Blocks.h"
#include "Camera.h"
#include "Entity.h"
#include "Player.h"
#include "Shader.h"
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

// Defining buffers.
UniformBuffer UBO;
Buffer OutlineBuffer;

// Variables for storing shader binding locations.
int ShaderTransparencyLoc;
int OutlineModelLoc;

// The main window which everything is rendered in.
GLFWwindow* Window = nullptr;

// The map where all the chunks are stored.
// Keys are the chunk's 3D-position.
// Values are pointers to the chunks.
std::map<glm::vec3, Chunk*, VectorComparator> ChunkMap;

// Defining options.
int RENDER_DISTANCE = 0;
int FULLSCREEN = 0;
int SCREEN_WIDTH = 0;
int SCREEN_HEIGHT = 0;
int VSYNC = 0;
int AMBIENT_OCCLUSION = 0;

// Defining shaders.
Shader* shader = nullptr;
Shader* modelShader = nullptr;
Shader* outlineShader = nullptr;
Shader* mobShader = nullptr;

// List of option references.
std::map<std::string, int*> Options = {
    {"RenderDistance", &RENDER_DISTANCE},
    {"FullScreen", &FULLSCREEN},
    {"WindowResX", &SCREEN_WIDTH},
    {"WindowResY", &SCREEN_HEIGHT},
    {"VSync", &VSYNC},
    {"AmbientOcclusion", &AMBIENT_OCCLUSION}
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

    // Parse the config file.
    Parse_Config();
    
    // Load all the block data.
    Blocks::Init();
    
    // Call the initializer functions.
    Init_GL();
    Init_Textures();
    Init_Shaders();
    Init_Outline();
    Init_Rendering();
    
    // Initialize the UI and the player.
    UI::Init();
    player.Init();
    
    // Start the background thread.
    std::thread chunkGeneration(Background_Thread);
    
    // Queues all surrounding chunks.
    player.Render_Chunks();
    
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
        
        // Checks if the game is in paused mode.
        if (!GamePaused) {
            // If not, check if any sounds should be removed.
            player.Poll_Sounds();
            
            // If mouse cursor isn't enabled, and chat isn't focused.
            if (!MouseEnabled && !chat.Focused) {
                // Update the player and block entities.
                player.Move(static_cast<float>(DeltaTime));
                Entity::Update(DeltaTime);
            }
            
            // Render the scene, and draw block entities and the player.
            Render_Scene();
            Entity::Draw();
            player.Draw();
        }
        
        // Draw the UI.
        UI::Draw();
        
        // If pressing L, take a screenshot of the screen buffer.
        if (keys[GLFW_KEY_L]) {
            Take_Screenshot();
        }
        
        // Swap the newly rendered frame with the old one.
        glfwSwapBuffers(Window);
    }
    
    // On shutting down, join the chunk generation thread with the main thread.
    chunkGeneration.join();
    
    // Shutdown the graphics library, and return.
    glfwTerminate();
    return 0;
}

// Sets the settings according to the config file.
void Parse_Config() {
    // String stream for holding the contents of the config file.
    std::stringstream file_content;
    
    // Load the config file, and store it in file_content.
    std::ifstream file(CONFIG_FILE);
    file_content << file.rdbuf();
    file.close();
    
    // String for storing every line.
    std::string line;
    
    // While there's new lines in the file.
    while (std::getline(file_content, line)) {
        // Get the position of the key-value divisor.
        int equalPos = static_cast<int>(line.find('='));
        
        // Get the key string.
        std::string key = line.substr(0, equalPos);
        
        // Checks if there's a key.
        if (key != "") {
            // If so, set the variable referenced in Options to the value.
            *Options[key] = std::stoi(line.substr(equalPos + 1));
        }
    }
}

// Write the settings to the config file.
void Write_Config() {
    // Open the config file for writing.
    std::ofstream file(CONFIG_FILE);
    
    // For each option, write it and its value to the file.
    for (auto const &option : Options) {
        file << option.first << "=" << *option.second << "\n";
    }
    
    // Close the file.
    file.close();
}

// Initialize OpenGL.
void Init_GL() {
    // Set the OpenGL version.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // Stops the window from being resizable.
    glfwWindowHint(GLFW_RESIZABLE, false);
    
    // Checks if Fullscreen was set to True in the config file.
    if (FULLSCREEN) {
        // Stops the window from having any decoration, such as a title bar.
        glfwWindowHint(GLFW_DECORATED, false);
        
        // Get the primary monitor.
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
    
    // Enable Depth Testing, which chooses which elements to draw based on their depth, thus allowing 3D to work.
    glEnable(GL_DEPTH_TEST);
    
    // Set the proper function for evaluating blending.
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Specify the size of the OpenGL view port.
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    
    // Set the color that the window gets filled with when the color buffer gets cleared.
    glClearColor(CLEAR_COLOR.r, CLEAR_COLOR.g, CLEAR_COLOR.b, 1.0f);
}

void Init_Textures() {
    // Set the active Texture Unit to 0.
    glActiveTexture(GL_TEXTURE0);
    
    // Load the texture atlas into a texture array, with mipmapping enabled, and store it in the active Texture Unit.
    glBindTexture(GL_TEXTURE_2D_ARRAY, Load_Array_Texture("atlas.png", glm::ivec2(16, 32), 4, true));
}

void Init_Shaders() {
    // Load the shaders.
    shader = new Shader("shader");
    outlineShader = new Shader("outline");
    modelShader = new Shader("model");
    mobShader = new Shader("model2DTex");
    
    // Create the frustrum projection matrix for the camera.
    glm::mat4 projection = glm::perspective(glm::radians(static_cast<float>(Cam.Zoom)), static_cast<float>(SCREEN_WIDTH) / SCREEN_HEIGHT, Z_NEAR_LIMIT, Z_FAR_LIMIT);
    
    // Create a matrix storage block in the shaders referenced in the last argument.
    UBO.Create("Matrices", 0, 2 * sizeof(glm::mat4), std::vector<Shader*> {shader, outlineShader, modelShader, mobShader});
    
    // Upload the projection matrix to index 1.
    UBO.Upload(1, projection);
    
    // Store locations of shader uniforms.
    ShaderTransparencyLoc = shader->Get_Location("RenderTransparent");
    OutlineModelLoc = outlineShader->Get_Location("model");
}

void Init_Outline() {
    // Vector for holding vertex data.
    Data data;
    
    // The X and Z-values for the vertices.
    int points[4][2] = { {0, 0}, {1, 0}, {1, 1}, {0, 1} };
    
    // The normal vector components, that helps offset the lines from the surface in order to avoid Z-fighting.
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
            Extend(data, points[i][0], yVec.x, points[i][1]);
            Extend(data, n[points[i][0]], n[yVec.x], n[points[i][1]]);
            Extend(data, points[index][0], yVec.y, points[index][1]);
            Extend(data, n[points[index][0]], n[yVec.y], n[points[index][1]]);
        }
    }
    
    // Initialize the outline buffer with its shader.
    OutlineBuffer.Init(outlineShader);
    
    // Set the vertex offsets, and upload the data.
    OutlineBuffer.Create(3, 3, data);
    
    // Set the vertex type of the buffer to lines.
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
    // Upload the camera's view matrix.
    UBO.Upload(0, Cam.GetViewMatrix());
    
    // If the wireframe option was toggled.
    if (ToggleWireframe) {
        // Toggle whether to use wireframe.
        Wireframe = !Wireframe;
        ToggleWireframe = false;
        
        // Toggle rendering mode.
        glPolygonMode(GL_FRONT_AND_BACK, Wireframe ? GL_LINE : GL_FILL);
        
        // If using wireframe, upload an invalid texture unit to make wireframe lines black.
        shader->Upload("diffTex", Wireframe ? 50 : 0);
    }
    
    // Set the first rendering pass to discard any transparent fragments.
    shader->Upload(ShaderTransparencyLoc, false);
    
    // Iterate through the chunks in the Chunk Map.
    for (auto const &chunk : ChunkMap) {
        // Checks if they have been meshed.
        if (chunk.second->Meshed) {
            // Uploads the data if it already hasn't been uploaded.
            if (!chunk.second->DataUploaded) {
                chunk.second->buffer.Upload(chunk.second->VBOData);
                chunk.second->DataUploaded = true;
            }
            
            // Draw the chunk if it is set to be visible.
            if (chunk.second->Visible) {
                chunk.second->buffer.Draw();
            }
        }
    }
    
    // Set the second rendering pass to discard any opaque fragments.
    shader->Upload(ShaderTransparencyLoc, true);
    
    // Iterate through the chunks again.
    for (auto const &chunk : ChunkMap) {
        // If the chunk is meshed, visible, and contains transparent blocks, render it.
        if (chunk.second->Meshed && chunk.second->Visible && chunk.second->ContainsTransparentBlocks) {
            chunk.second->buffer.Draw();
        }
    }
    
    // Checks if the player is currently looking at a block.
    if (player.LookingAtBlock) {
        // Get the type and data of the block.
        int blockType = ChunkMap[player.LookingChunk]->Get_Block(player.LookingTile);
        int blockData = ChunkMap[player.LookingChunk]->Get_Block(player.LookingTile);
        
        // Start with an empty identity matrix.
        glm::mat4 model;
        
        // WIP multi-block outline.
        if (blockData == -1) {
            const Block* block = Blocks::Get_Block(blockType);
            model = glm::translate(model, Get_World_Pos(player.LookingChunk, player.LookingTile) + block->ScaleOffset - glm::vec3(0, 1, 0));
            model = glm::scale(model, block->Scale);
        }
        else {
            // Translate the outline, and scale it to the block size.
            model = glm::translate(model, Get_World_Pos(player.LookingChunk, player.LookingTile) + player.LookingBlockType->ScaleOffset);
            model = glm::scale(model, player.LookingBlockType->Scale);
        }
        
        // Upload the matrix, and draw the outline.
        outlineShader->Upload(OutlineModelLoc, model);
        OutlineBuffer.Draw();
    }
}

void Background_Thread() {
    // Start the thread loop.
    while (true) {
        // Set the thread to return if the window is closed.
        if (glfwWindowShouldClose(Window)) {
            return;
        }
        
        // Variable to check if the queue is empty, defaults to true.
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
        
        // Iterates through the chunks.
        for (auto const &chunk : ChunkMap) {
            // Checks if they haven't already been meshed.
            if (!chunk.second->Meshed) {
                // Get the distance between the chunk and the player's position.
                float dist = glm::distance(chunk.first.xz(), playerPos);
                
                // If the distance is smaller than the smallest so far, set the chunk to be the nearest chunk.
                if (dist < nearestDistance && dist < RENDER_DISTANCE) {
                    nearestDistance = dist;
                    nearestChunk = chunk.second;
                }
            }
        }
        
        // Checks if there's a chunk to be rendered.
        if (nearestChunk != nullptr) {
            // Generates it if it already hasn't been generated.
            if (!nearestChunk->Generated) {
                nearestChunk->Generate();
            }
            
            // Light and mesh it.
            nearestChunk->Light();
            nearestChunk->Mesh();
            
            // Flag the chunk as meshed.
            nearestChunk->Meshed = true;
            
            // Flag the chunk to indicate that its data need to be uploaded.
            nearestChunk->DataUploaded = false;
            
            // Show that the queue isn't empty.
            queueEmpty = false;
        }
        
        // Show that the thread is no longer using the chunk map.
        ChunkMapBusy = false;
        
        // Sleep for 1 ms if there's still chunks to be generated, else sleep for 100 ms.
        std::this_thread::sleep_for(std::chrono::milliseconds(queueEmpty ? 100 : 1));
    }
}

// Proxy for getting key events.
void key_proxy(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // If the game isn't paused.
    if (!GamePaused) {
        // If the chat is focused, clear the list of currently pressed keys.
        if (chat.Focused) {
            player.Clear_Keys();
        }
        // Else, send the key event to the UI and Player key handlers.
        else {
            UI::Key_Handler(key, action);
            player.Key_Handler(key, action);
        }
        
        // If a key is being pressed down, send the key to the chat key handler.
        if (action == GLFW_PRESS) {
            chat.Key_Handler(key);
        }
    }
    // If the game is paused, send the key event to the UI key handler.
    else {
        UI::Key_Handler(key, action);
    }
}
// Proxy for receiving Unicode codepoints, very useful for getting text input.
void text_proxy(GLFWwindow* window, unsigned int codepoint) {
    // If the chat is focused and chat hasn't been toggled, send the codepoint to the chat.
    if (chat.Focused && !chat.FocusToggled) {
        chat.Input(codepoint);
    }
}
// Proxy for getting mouse movement.
void mouse_proxy(GLFWwindow* window, double posX, double posY) {
    // Send the new mouse coordinates to the UI mouse handler.
    UI::Mouse_Handler(posX, posY);
    
    // If the game isn't paused and chat isn't focused, send the coordinates to the player mouse handler.
    if (!GamePaused && !chat.Focused) {
        player.Mouse_Handler(posX, posY);
    }
}
// Proxy for getting scrolling events.
void scroll_proxy(GLFWwindow* window, double offsetX, double offsetY) {
    // If the game isn't paused, send the vertical scrolling offset to the player scroll handler.
    if (!GamePaused) {
        player.Scroll_Handler(offsetY);
    }
}
// Proxy for getting mouse clicking events.
void click_proxy(GLFWwindow* window, int button, int action, int mods) {
    // Send the click event to the UI click handler.
    UI::Click(player.LastMousePos.x, player.LastMousePos.y, action, button);
    
    // If the game isn't paused, send the click event to the player click handler.
    if (!GamePaused) {
        player.Click_Handler(button, action);
    }
}

void Exit() {
    glfwSetWindowShouldClose(Window, GL_TRUE);
}