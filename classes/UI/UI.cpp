#include "UI.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "System.h"

const int FONT_SIZE = 15;

const int AVG_UPDATE_RANGE = 10;
const double UI_UPDATE_FREQUENCY = 1.0;

const std::string FONT = "Roboto";

double last_fps[AVG_UPDATE_RANGE];
double last_cpu[AVG_UPDATE_RANGE];
double lastUIUpdate;

bool ShowInventory = false;
bool ShowMenu = false;
bool ShowDebug = false;
bool ShowOptions = false;

int colorLocation;
int alphaLocation;
int borderColorLocation;

unsigned int BackgroundVAO, BackgroundVBO;
Buffer BackgroundBuffer;

glm::vec3 BackgroundColor = glm::vec3(0);
float BackgroundOpacity = 0.5f;

std::string BoolStrings[2] = {"False", "True"};

void Toggle_Mouse(bool enable);

void Init_UI_Shaders();
void Init_UI();
void Init_Background();
void Init_Menu();
void Init_Debug();

void Draw_UI();
void Draw_Background();
void Draw_Menu();
void Draw_Debug();

void Toggle_Options_Menu();
void Toggle_VSync();
void Toggle_Wireframe();

void Change_Render_Distance();

void Exit();

void UI::Init() {
    Text::Init(FONT, FONT_SIZE);
    
    Init_UI_Shaders();
    Init_UI();
    Init_Background();
    Init_Menu();
    Init_Debug();
    
    player.inventory.Mesh();
}

void UI::Draw() {
    if (Wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    
    if (ShowMenu) {
        Draw_Background();
        Draw_Menu();
    }
    else {
        Draw_UI();
        
        if (ShowDebug) {
            Draw_Debug();
        }
        else {
            player.inventory.Draw();
        }
    }
    
    if (Wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
}

void UI::Clean() {
    delete UIShader;
    delete UIBorderShader;
    delete UITextureShader;
    
    UIShader = nullptr;
    UIBorderShader = nullptr;
    UITextureShader = nullptr;
}

void UI::Click(double mouseX, double mouseY, int action, int button) {
    if (ShowMenu && button == GLFW_MOUSE_BUTTON_LEFT) {
        Button::Check_Click(mouseX, SCREEN_HEIGHT - mouseY, action);
        Slider::Check_Click(mouseX, SCREEN_HEIGHT - mouseY, action);
    }
    
    if (ShowInventory && action == GLFW_PRESS) {
        player.inventory.Click_Handler(mouseX, mouseY, button);
    }
}

void UI::Toggle_Menu() {
    ShowMenu = !ShowMenu;
    ShowOptions = false;
    
    if (!ShowInventory) {
        Toggle_Mouse(ShowMenu);
    }
}

void UI::Toggle_Debug() {
    ShowDebug = !ShowDebug;
}

void UI::Toggle_Inventory() {
    ShowInventory = !ShowInventory;
    Toggle_Mouse(ShowInventory);
}

void Toggle_Mouse(bool enable) {
    MouseEnabled = enable;
    
    if (enable) {
        glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    else {
        glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}

void Init_UI_Shaders() {
    UIShader = new Shader("ui");
    UIBorderShader = new Shader("uiBorder");
    UITextureShader = new Shader("uiTex");
    
    colorLocation = glGetUniformLocation(UIShader->Program, "color");
    alphaLocation = glGetUniformLocation(UIShader->Program, "alpha");
    borderColorLocation = glGetUniformLocation(UIBorderShader->Program, "color");
    
    glm::mat4 projection = glm::ortho(0.0f, (float)SCREEN_WIDTH, 0.0f, (float)SCREEN_HEIGHT);
    
    UIShader->Upload("projection", projection);
    
    UIBorderShader->Upload("projection", projection);
    UIBorderShader->Upload(borderColorLocation, glm::vec3(0));
    
    UITextureShader->Upload("projection", projection);
    UITextureShader->Upload("tex", 0);
}

void Init_UI() {
    chat.Init(*UIShader, *UIBorderShader, colorLocation, alphaLocation);
    player.inventory.Init();
}

void Init_Background() {
    float w = float(SCREEN_WIDTH);
    float h = float(SCREEN_HEIGHT);
    
    BackgroundBuffer.Init(UIShader);
    BackgroundBuffer.Create(std::vector<int> {2}, Data {0, 0,  w, 0,  w, h,  0, 0,  w, h,  0, h});
}

void Init_Menu() {
    float buttonWidth = X_Frac(5, 36);
    
    float optionsButtonX = X_Frac(31, 72);
    float optionsButtonY = Y_Frac(5, 9);
    
    float exitButtonX = X_Frac(31, 72);
    float exitButtonY = Y_Frac(2, 9);
    
    float vSyncButtonX = X_Frac(7, 24);
    float vSyncButtonY = Y_Frac(5, 9);
    
    float wireframeButtonX = X_Frac(13, 24);
    float wireframeButtonY = Y_Frac(5, 9);
    
    float renderDistButtonX = X_Frac(31, 72);
    float renderDistButtonY = Y_Frac(7, 9);
    
    float backButtonX = X_Frac(31, 72);
    float backButtonY = Y_Frac(2, 9);
    
    Button::Add("options", "Options", Toggle_Options_Menu, optionsButtonX, optionsButtonY, buttonWidth, "mainMenu");
    Button::Add("exit", "Quit", Exit, exitButtonX, exitButtonY, buttonWidth, "mainMenu");
    
    Button::Add("option_vsync", "V-Sync: " + BoolStrings[VSync], Toggle_VSync, vSyncButtonX, vSyncButtonY, buttonWidth, "optionMenu");
    Button::Add("option_wireframe", "Wireframe: " + BoolStrings[Wireframe], Toggle_Wireframe, wireframeButtonX, wireframeButtonY, buttonWidth, "optionMenu");
    
    Slider::Add("option_renderDistance", "Render Distance: " + std::to_string(RenderDistance), Change_Render_Distance, renderDistButtonX, renderDistButtonY, buttonWidth, 0, 10, float(RenderDistance), "optionMenu");
    
    Button::Add("option_back", "Back", Toggle_Options_Menu, backButtonX, backButtonY, buttonWidth, "optionMenu");
}

void Init_Debug() {
    lastUIUpdate = glfwGetTime();
    
    float debugX = X_Frac(1, 48);
    
    float fpsY = Y_Frac(17, 18);
    float cpuY = Y_Frac(41, 45);
    float ramY = Y_Frac(5, 6);
    float virtualY = Y_Frac(4, 5);
    float chunkY = Y_Frac(34, 45);
    float tileY = Y_Frac(13, 18);
    float posY = Y_Frac(31, 45);
    float queueY = Y_Frac(29, 45);
    
    Text::Set_Group("debug");
    Text::Set_X("debug", debugX);
    
    Text::Add("fps", "FPS: 0", fpsY);
    Text::Add("cpu", "CPU: 0%", cpuY);
    Text::Add("ram", "RAM: " + System::GetPhysicalMemoryUsage(), ramY);
    Text::Add("virtualMemory", "Virtual Memory: " + System::GetVirtualMemoryUsage(), virtualY);
    Text::Add("playerChunk", "Chunk:    ", chunkY);
    Text::Add("playerTile", "Tile:     ", tileY);
    Text::Add("playerPos", "Position: ", posY);
    Text::Add("chunkQueue", "Chunks Loaded: ", queueY);
    
    if (Windows) {
        float vramY = Y_Frac(13, 15);
        Text::Add("vram", "VRAM: " + System::GetVRAMUsage(), vramY);
    }
    
    Text::Unset_Group();
}

void Draw_UI() {
    chat.Update();
}

void Draw_Background() {
    UIShader->Upload(colorLocation, BackgroundColor);
    UIShader->Upload(alphaLocation, BackgroundOpacity);
    
    BackgroundBuffer.Draw();
    
    glClear(GL_DEPTH_BUFFER_BIT);
}

void Draw_Menu() {
    Button::Check_Hover(player.LastMousePos.x, SCREEN_HEIGHT - player.LastMousePos.y);
    Slider::Check_Hover(player.LastMousePos.x, SCREEN_HEIGHT - player.LastMousePos.y);
    
    if (ShowOptions) {
        Button::Draw_All("optionMenu");
        Slider::Draw_All("optionMenu");
    }
    else {
        Button::Draw_All("mainMenu");
        Slider::Draw_All("mainMenu");
    }
}

int Get_Loaded() {
    int total = 0;
    
    for (auto const &chunk : ChunkMap) {
        total += chunk.second->Meshed;
    }
    
    return total;
}

void Draw_Debug() {
    for (int i = 0; i < AVG_UPDATE_RANGE; i++) {
        if (i < AVG_UPDATE_RANGE - 1) {
            last_fps[i] = last_fps[i + 1];
            last_cpu[i] = last_cpu[i + 1];
        }
        else {
            last_fps[i] = (1.0f / DeltaTime + 0.5);
            last_cpu[i] = System::GetCPUUsage();
        }
    }
    
    if (LastFrame - lastUIUpdate >= UI_UPDATE_FREQUENCY) {
        lastUIUpdate = LastFrame;
        
        double fps_sum = 0.0;
        double cpu_sum = 0.0;
        
        for (int i = 0; i < AVG_UPDATE_RANGE; i++) {
            fps_sum += last_fps[i];
            cpu_sum += last_cpu[i];
        }
        
        Text::Set_Group("debug");
        
        Text::Set_Text("fps", "FPS: " + std::to_string((int)(fps_sum / AVG_UPDATE_RANGE)));
        Text::Set_Text("cpu", "CPU: " + std::to_string((int)(cpu_sum / AVG_UPDATE_RANGE)) + "%");
        Text::Set_Text("ram", "RAM: " + System::GetPhysicalMemoryUsage());
        Text::Set_Text("virtualMemory", "Virtual Memory: " + System::GetVirtualMemoryUsage());
        
        if (Windows) {
            Text::Set_Text("vram", "VRAM: " + System::GetVRAMUsage());
        }
    }
    
    Text::Set_Group("debug");
    
    Text::Set_Text("playerChunk", "Chunk:      " + Format_Vector(player.CurrentChunk));
    Text::Set_Text("playerTile", "Tile:            " + Format_Vector(player.CurrentTile));
    Text::Set_Text("playerPos", "Position:  " + Format_Vector(player.WorldPos));
    Text::Set_Text("chunkQueue", "Chunks Queued: " + std::to_string(int(ChunkMap.size()) - Get_Loaded()));
    
    Text::Unset_Group();
    
    Text::Draw_Group("debug");
}

void Toggle_Options_Menu() {
    ShowOptions = !ShowOptions;
}

void Toggle_VSync() {
    VSync = !VSync;
    Button::Set_Text("option_vsync", "V-Sync: " + BoolStrings[VSync]);
    
    glfwSwapInterval(VSync);
    Write_Config();
}

void Toggle_Wireframe() {
    ToggleWireframe = true;
    Button::Set_Text("option_wireframe", "Wireframe: " + BoolStrings[!Wireframe]);
}

void Change_Render_Distance() {
    int value = int(ceil(Slider::Get_Value("option_renderDistance")));
    
    if (value != RenderDistance) {
        RenderDistance = value;
        Slider::Set_Text("option_renderDistance", "Render Distance: " + std::to_string(value));
        Write_Config();
        
        player.RenderChunks();
    }
}

void Exit() {
    glfwSetWindowShouldClose(Window, GL_TRUE);
}

std::string Format_Vector(glm::vec3 vector) {
    std::string x = std::to_string(int(vector.x));
    std::string y = std::to_string(int(vector.y));
    std::string z = std::to_string(int(vector.z));
    
    return std::string("X: " + x + "\t\tY: " + y + "\t\tZ: " + z);
}