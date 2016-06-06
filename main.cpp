#include "main.h"

#include <sstream>
#include <fstream>
#include <dirent.h>

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

Player player;
Chat chat = Chat();
Camera Cam = Camera();
Listener listener = Listener();
Interface interface = Interface();
Inventory inventory = Inventory();

Shader* shader;
Shader* modelShader;
Shader* outlineShader;
Shader* mobShader;

UniformBuffer UBO;
Buffer OutlineBuffer;

int ShaderTransparencyLoc;
int OutlineModelLoc;

GLFWwindow* Window;

std::map<glm::vec3, Chunk*, Vec3Comparator> ChunkMap;

void Parse_Config();

void Init_GL();
void Init_Textures();
void Init_Shaders();
void Init_Buffers();
void Init_Rendering();

void Render_Scene();

void BackgroundThread();

void key_proxy(GLFWwindow* window, int key, int scancode, int action, int mods);
void text_proxy(GLFWwindow* window, unsigned int codepoint);
void mouse_proxy(GLFWwindow* window, double posX, double posY);
void scroll_proxy(GLFWwindow* window, double xoffset, double yoffset);
void click_proxy(GLFWwindow* window, int button, int action, int mods);

int main() {
    glfwInit();
    
    Parse_Config();
    Blocks::Init();
    
	Init_GL();
	Init_Textures();
    
	Init_Shaders();
    Init_Buffers();
	Init_Rendering();
    
    UI::Init();
    player.Init();
    
	std::thread chunkGeneration(BackgroundThread);
    
    player.Render_Chunks();
    
	while (!glfwWindowShouldClose(Window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
		double currentFrame = glfwGetTime();
		DeltaTime = currentFrame - LastFrame;
		LastFrame = currentFrame;

		glfwPollEvents();
        
        if (!GamePaused) {
            player.Poll_Sounds();
            
            if (!MouseEnabled && !chat.Focused) {
                player.Move(float(DeltaTime));
                Entity::Update(DeltaTime);
            }
            
            Render_Scene();
            Entity::Draw();
            player.Draw();
        }
        
        UI::Draw();
        
        if (keys[GLFW_KEY_L]) {
            Take_Screenshot();
        }
        
		glfwSwapBuffers(Window);
	}
    
    chunkGeneration.join();
    
    glfwTerminate();
    return 0;
}

void Parse_Config() {
    std::stringstream file_content;
    
    std::ifstream file(CONFIG_FILE);
    file_content << file.rdbuf();
    file.close();
    
    std::string line;
    
    while (std::getline(file_content, line)) {
        std::istringstream is_line(line);
        std::string key, value;
        
        std::getline(is_line, key, '=');
        std::getline(is_line, value);
        
        if (key != "") {
            *Options[key] = std::stoi(value);
        }
    }
}

void Write_Config() {
    std::ofstream file(CONFIG_FILE);
    
    for (auto const &option : Options) {
        file << option.first << "=" << *option.second << "\n";
    }
    
    file.close();
}

void Init_GL() {
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    glfwWindowHint(GLFW_RESIZABLE, false);
    
    if (Fullscreen) {
        glfwWindowHint(GLFW_DECORATED, false);
        
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);
        
        SCREEN_WIDTH = videoMode->width;
        SCREEN_HEIGHT = videoMode->height;
        
        Window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Craftmine", monitor, nullptr);
    }
    else {
        glfwWindowHint(GLFW_DECORATED, true);
        Window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Craftmine", nullptr, nullptr);
    }

	glfwSetWindowPos(Window, 0, 0);

	glfwMakeContextCurrent(Window);
	glfwSwapInterval(VSync);

	glewExperimental = GL_TRUE;
	glewInit();

	glfwSetKeyCallback(Window, key_proxy);
	glfwSetCursorPosCallback(Window, mouse_proxy);
	glfwSetScrollCallback(Window, scroll_proxy);
	glfwSetMouseButtonCallback(Window, click_proxy);
    glfwSetCharCallback(Window, text_proxy);

	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	glClearColor(CLEAR_COLOR.r, CLEAR_COLOR.g, CLEAR_COLOR.b, 1.0f);
}

void Init_Textures() {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, Load_Array_Texture("atlas.png", glm::ivec2(16), 4, true));
}

void Init_Shaders() {
    shader = new Shader("shader");
    outlineShader = new Shader("outline");
    modelShader = new Shader("model");
    mobShader = new Shader("model2DTex");
    
    glm::mat4 projection = glm::perspective(glm::radians((float)Cam.Zoom), (float)SCREEN_WIDTH / SCREEN_HEIGHT, 0.04f, 1000.0f);
    UBO.Create("Matrices", 0, 2 * sizeof(glm::mat4), std::vector<Shader*> {shader, outlineShader, modelShader, mobShader});
    UBO.Upload(1, projection);
    
    ShaderTransparencyLoc = shader->Get_Location("RenderTransparent");
    OutlineModelLoc = outlineShader->Get_Location("model");
}

void Init_Buffers() {
    Data data;
    
    int points[4][2] = { {0, 0}, {1, 0}, {1, 1}, {0, 1} };
    float n[2] {-1 / sqrtf(3), 1 / sqrtf(3)};
    
    for (int y = 0; y < 3; y++) {
        for (int i = 0; i < 4; i++) {
            int x1 = points[i % 4][0];
            int z1 = points[i % 4][1];
            
            int y1 = (y == 2) ? 0 : y;
            int y2 = (y == 2) ? 1 : y;
            
            int x2 = (y == 2) ? x1 : ((i != 4) ? points[(i + 1) % 4][0] : 0);
            int z2 = (y == 2) ? z1 : ((i != 4) ? points[(i + 1) % 4][1] : 0);
            
            Extend(data, x1, y1, z1);
            Extend(data, n[x1], n[y1], n[z1]);
            
            Extend(data, x2, y2, z2);
            Extend(data, n[x2], n[y2], n[z2]);
        }
    }
    
    OutlineBuffer.Init(outlineShader);
    OutlineBuffer.Create(3, 3, data);
    OutlineBuffer.VertexType = GL_LINES;
}

void Init_Rendering() {
    glm::mat4 model;
    
    shader->Upload("model", model);
    shader->Upload("ambient", AMBIENT_LIGHT);
    shader->Upload("diffuse", DIFFUSE_LIGHT);
    shader->Upload("diffTex", 0);
    
    modelShader->Upload("tex", 0);
}

void Render_Scene() {
    UBO.Upload(0, Cam.GetViewMatrix());
    
	if (ToggleWireframe) {
		Wireframe = !Wireframe;
		ToggleWireframe = false;
        
        glPolygonMode(GL_FRONT_AND_BACK, Wireframe ? GL_LINE : GL_FILL);
        shader->Upload("diffTex", Wireframe ? 50 : 0);
	}
    
    shader->Upload(ShaderTransparencyLoc, false);
    
    for (auto const &chunk : ChunkMap) {
        if (chunk.second->Meshed) {
            if (!chunk.second->DataUploaded) {
                chunk.second->buffer.Upload(chunk.second->VBOData);
                chunk.second->DataUploaded = true;
            }
            
            if (chunk.second->Visible) {
                chunk.second->buffer.Draw();
            }
        }
	}
    
    shader->Upload(ShaderTransparencyLoc, true);
    
    for (auto const &chunk : ChunkMap) {
        if (chunk.second->Meshed && chunk.second->Visible && chunk.second->ContainsTransparentBlocks) {
            chunk.second->buffer.Draw();
        }
    }
    
    if (player.LookingAtBlock) {
        int blockType = ChunkMap[player.LookingChunk]->Get_Block(player.LookingTile);
        int blockData = ChunkMap[player.LookingChunk]->Get_Block(player.LookingTile);
        
        glm::mat4 model;
        
        if (blockData == -1) {
            const Block* block = Blocks::Get_Block(blockType);
            model = glm::translate(model, Get_World_Pos(player.LookingChunk, player.LookingTile) + block->ScaleOffset - glm::vec3(0, 1, 0));
            model = glm::scale(model, block->Scale);
        }
        else {
            model = glm::translate(model, Get_World_Pos(player.LookingChunk, player.LookingTile) + player.LookingBlockType->ScaleOffset);
            model = glm::scale(model, player.LookingBlockType->Scale);
        }
        
        outlineShader->Upload(OutlineModelLoc, model);
        OutlineBuffer.Draw();
    }
}

void BackgroundThread() {
	while (true) {
        if (glfwWindowShouldClose(Window)) {
            return;
        }
        
        bool queueEmpty = true;
        
        while (ChunkMapBusy) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        
        ChunkMapBusy = true;
        
        glm::vec2 playerPos = player.CurrentChunk.xz();
        float nearestDistance = RenderDistance;
        Chunk* nearestChunk = nullptr;
        
        for (auto const &chunk : ChunkMap) {
            if (!chunk.second->Meshed) {
                float dist = glm::distance(chunk.first.xz(), playerPos);
                
                if (dist < nearestDistance && dist < RenderDistance) {
                    nearestDistance = dist;
                    nearestChunk = chunk.second;
                }
            }
        }
        
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
        
        ChunkMapBusy = false;
        std::this_thread::sleep_for(std::chrono::milliseconds(queueEmpty ? 100 : 1));
	}
}

void key_proxy(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (!GamePaused) {
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
    else {
        UI::Key_Handler(key, action);
    }
}
void text_proxy(GLFWwindow* window, unsigned int codepoint) {
    if (!GamePaused && chat.Focused && !chat.FocusToggled) {
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
    UI::Click(player.LastMousePos.x, player.LastMousePos.y, action, button);
    
    if (!GamePaused) {
        player.Click_Handler(button, action);
    }
}