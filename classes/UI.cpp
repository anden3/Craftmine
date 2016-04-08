#include "UI.h"

#include "System.h"
#include "Button.h"

const int AVG_UPDATE_RANGE = 10;
const double UI_UPDATE_FREQUENCY = 1.0;

const std::string FONT = "Roboto";

double last_fps[AVG_UPDATE_RANGE];
double last_cpu[AVG_UPDATE_RANGE];
double lastUIUpdate;

bool ShowMenu = false;
bool ShowDebug = false;
bool ShowOptions = false;

std::string BoolStrings[2] = {"False", "True"};

void Init_UI();
void Init_Menu();
void Init_Debug();

void Draw_UI();
void Draw_Menu();
void Draw_Debug();

void Toggle_Options_Menu();
void Toggle_VSync();
void Toggle_Wireframe();

void Exit();


void UI::Init() {
    Text::Init(FONT, FONT_SIZE);
    
    Init_UI();
    Init_Menu();
    Init_Debug();
}

void UI::Draw() {
    if (Wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    if (ShowMenu) {
        Draw_Menu();
    }
    else {
        Draw_UI();
        
        if (ShowDebug) {
            Draw_Debug();
        }
    }
    
    if (Wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void UI::Clean() {
    Button::Clean();
}

void UI::Toggle_Menu() {
    ShowMenu = !ShowMenu;
    ShowOptions = false;
    
    if (ShowMenu) glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    else glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void UI::Toggle_Debug() {
    ShowDebug = !ShowDebug;
}

void Init_UI() {
    return;
}

void Init_Menu() {
    Button::Add("options", "Options", Toggle_Options_Menu, 620, 500, 200, "mainMenu");
    Button::Add("exit", "Quit", Exit, 620, 200, 200, "mainMenu");
    
    Button::Add("option_vsync", "V-Sync: " + BoolStrings[VSYNC], Toggle_VSync, 420, 500, 200, "optionMenu");
    Button::Add("option_wireframe", "Wireframe: " + BoolStrings[Wireframe], Toggle_Wireframe, 780, 500, 200, "optionMenu");
    
    Button::Add("option_back", "Back", Toggle_Options_Menu, 620, 200, 200, "optionMenu");
}

void Init_Debug() {
    lastUIUpdate = glfwGetTime();
    
    Text::Set_Group("debug");
    Text::Set_X("debug", 30);
    
    Text::Add("fps", "FPS: 0", SCREEN_HEIGHT - 50);
    Text::Add("cpu", "CPU: 0%", SCREEN_HEIGHT - 80);
    
    if (Windows) {
        Text::Add("vram", "VRAM: " + System::GetVRAMUsage(), SCREEN_HEIGHT - 120);
    }
    
    Text::Add("ram", "RAM: " + System::GetPhysicalMemoryUsage(), SCREEN_HEIGHT - 150);
    Text::Add("virtualMemory", "Virtual Memory: " + System::GetVirtualMemoryUsage(), SCREEN_HEIGHT - 180);
    
    Text::Add("playerChunk", "Chunk:    ", SCREEN_HEIGHT - 220);
    Text::Add("playerTile", "Tile:     ", SCREEN_HEIGHT - 250);
    Text::Add("playerPos", "Position: ", SCREEN_HEIGHT - 280);
    
    Text::Add("chunkQueue", "Chunks Queued: ", SCREEN_HEIGHT - 320);
    
    Text::Unset_Group();
}

void Draw_UI() {
    return;
}

void Draw_Menu() {
    Button::Check_Hover(player.LastMousePos.x, SCREEN_HEIGHT - player.LastMousePos.y);
    
    if (ShowOptions) {
        Button::Draw_All("optionMenu");
    }
    else {
        Button::Draw_All("mainMenu");
    }
}

void Draw_Debug() {
    for (int i = 0; i < AVG_UPDATE_RANGE; i++) {
        if (i < AVG_UPDATE_RANGE - 1) {
            last_fps[i] = last_fps[i + 1];
            last_cpu[i] = last_cpu[i + 1];
        }
        else {
            last_fps[i] = (1.0f / deltaTime + 0.5);
            last_cpu[i] = System::GetCPUUsage();
        }
    }
    
    if (lastFrame - lastUIUpdate >= UI_UPDATE_FREQUENCY) {
        lastUIUpdate = lastFrame;
        
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
        
        if (Windows) Text::Set_Text("vram", "VRAM: " + System::GetVRAMUsage());
    }
    
    Text::Set_Group("debug");
    
    Text::Set_Text("playerChunk", "Chunk:      " + Format_Vector(player.CurrentChunk));
    Text::Set_Text("playerTile", "Tile:            " + Format_Vector(player.CurrentTile));
    Text::Set_Text("playerPos", "Position:  " + Format_Vector(player.WorldPos));
    Text::Set_Text("chunkQueue", "Chunks Queued: " + std::to_string(ChunkQueue.size()));
    
    Text::Unset_Group();
    
    Text::Draw_Group("debug");
}

void Toggle_Options_Menu() {
    ShowOptions = !ShowOptions;
}

void Toggle_VSync() {
    VSYNC = !VSYNC;
    Button::Set_Text("option_vsync", "V-Sync: " + BoolStrings[VSYNC]);
    
    glfwSwapInterval(VSYNC);
}

void Toggle_Wireframe() {
    ToggleWireframe = true;
    Button::Set_Text("option_wireframe", "Wireframe: " + BoolStrings[!Wireframe]);
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