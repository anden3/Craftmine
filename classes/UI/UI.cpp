#include "UI.h"

#include "System.h"
#include "Interface.h"

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
    interface.Init();
    player.inventory.Init();
    chat.Init();
    
    Init_Menu();
    Init_Debug();
    
    // player.inventory.Mesh();
}

void UI::Draw() {
    if (Wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    
    if (ShowMenu) {
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

void UI::Click(double mouseX, double mouseY, int action, int button) {
    if (ShowMenu) {
        interface.Click(button, action);
    }
    
    if (ShowInventory) {
        player.inventory.Click_Handler(mouseX, mouseY, button, action);
    }
}

void UI::Mouse_Handler(double x, double y) {
    if (ShowMenu) {
        if (ShowOptions) {
            interface.Set_Document("options");
        }
        else {
            interface.Set_Document("mainMenu");
        }
        
        interface.Mouse_Handler(float(x), float(SCREEN_HEIGHT - y));
        interface.Set_Document("");
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
    
    float w = float(SCREEN_WIDTH);
    float h = float(SCREEN_HEIGHT);
    
    interface.Set_Document("mainMenu");
    
    interface.Add_Background("menuBg", 0, 0, w, h);
    interface.Add_Button("options", "Options", optionsButtonX, optionsButtonY, buttonWidth, Toggle_Options_Menu);
    interface.Add_Button("exit", "Quit", exitButtonX, exitButtonY, buttonWidth, Exit);
    
    interface.Set_Document("options");
    
    interface.Add_Background("menuBg", 0, 0, w, h);
    interface.Add_Button("option_vsync", "V-Sync: " + BoolStrings[VSync], vSyncButtonX, vSyncButtonY, buttonWidth, Toggle_VSync);
    interface.Add_Button("option_wireframe", "Wireframe: " + BoolStrings[Wireframe], wireframeButtonX, wireframeButtonY, buttonWidth, Toggle_Wireframe);
    interface.Add_Slider("option_renderDistance", "Render Distance: " + std::to_string(RenderDistance), renderDistButtonX, renderDistButtonY, buttonWidth, 0, 10, float(RenderDistance), Change_Render_Distance);
    interface.Add_Button("option_back", "Back", backButtonX, backButtonY, buttonWidth, Toggle_Options_Menu);
    
    interface.Set_Document("");
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
    
    interface.Set_Document("debug");
    
    interface.Add_Text("fps", "FPS: 0", debugX, fpsY);
    interface.Add_Text("cpu", "CPU: 0%", debugX, cpuY);
    interface.Add_Text("ram", "RAM: " + System::GetPhysicalMemoryUsage(), debugX, ramY);
    interface.Add_Text("virtualMemory", "Virtual Memory: " + System::GetVirtualMemoryUsage(), debugX, virtualY);
    interface.Add_Text("playerChunk", "Chunk:    ", debugX, chunkY);
    interface.Add_Text("playerTile", "Tile:     ", debugX, tileY);
    interface.Add_Text("playerPos", "Position: ", debugX, posY);
    interface.Add_Text("chunkQueue", "Chunks Loaded: ", debugX, queueY);
    
    if (Windows) {
        float vramY = Y_Frac(13, 15);
        interface.Add_Text("vram", "VRAM: " + System::GetVRAMUsage(), debugX, vramY);
    }
    
    interface.Set_Document("");
}

void Draw_UI() {
    chat.Update();
}

void Draw_Menu() {
    interface.Mouse_Handler(player.LastMousePos.x, player.LastMousePos.y);
    
    if (ShowOptions) {
        interface.Draw_Document("options");
    }
    else {
        interface.Draw_Document("mainMenu");
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
        
        interface.Set_Document("debug");
        
        interface.Get_Text_Element("fps")->Text = "FPS: " + std::to_string((int)(fps_sum / AVG_UPDATE_RANGE));
        interface.Get_Text_Element("cpu")->Text = "CPU: " + std::to_string((int)(cpu_sum / AVG_UPDATE_RANGE)) + "%";
        interface.Get_Text_Element("ram")->Text = "RAM: " + System::GetPhysicalMemoryUsage();
        interface.Get_Text_Element("virtualMemory")->Text = "Virtual Memory: " + System::GetVirtualMemoryUsage();
        
        if (Windows) {
            interface.Get_Text_Element("vram")->Text = "VRAM: " + System::GetVRAMUsage();
        }
    }
    
    interface.Set_Document("debug");
    
    interface.Get_Text_Element("playerChunk")->Text = "Chunk:      " + Format_Vector(player.CurrentChunk);
    interface.Get_Text_Element("playerTile")->Text = "Tile:            " + Format_Vector(player.CurrentTile);
    interface.Get_Text_Element("playerPos")->Text = "Position:  " + Format_Vector(player.WorldPos);
    interface.Get_Text_Element("chunkQueue")->Text = "Chunks Queued: " + std::to_string(int(ChunkMap.size()) - Get_Loaded());
    
    interface.Set_Document("");
    
    interface.Draw_Document("debug");
}

void Toggle_Options_Menu() {
    ShowOptions = !ShowOptions;
}

void Toggle_VSync() {
    VSync = !VSync;
    interface.Get_Button("option_vsync")->Text.Text = "V-Sync: " + BoolStrings[VSync];
    
    glfwSwapInterval(VSync);
    Write_Config();
}

void Toggle_Wireframe() {
    ToggleWireframe = true;
    interface.Get_Button("option_wireframe")->Text.Text = "Wireframe: " + BoolStrings[!Wireframe];
}

void Change_Render_Distance() {
    Slider* slider = interface.Get_Slider("option_renderDistance");
    
    int value = int(ceil(slider->Value));
    
    if (value != RenderDistance) {
        RenderDistance = value;
        slider->Text.Text = "Render Distance: " + std::to_string(value);
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