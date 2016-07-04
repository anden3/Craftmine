#include "UI.h"

#include "Chat.h"
#include "main.h"
#include "Chunk.h"
#include "Player.h"
#include "System.h"
#include "Interface.h"
#include "Inventory.h"

const int AVG_UPDATE_RANGE = 10;
const double UI_UPDATE_FREQUENCY = 1.0;

const std::string FONT = "Roboto";

static double lastUIUpdate;
static std::deque<int> CPU;

static bool ShowTitle = true;
static bool ShowInventory = false;
static bool ShowGameMenu = false;
static bool ShowDebug = false;
static bool ShowOptions = false;

const std::string BoolStrings[2] = {"False", "True"};

void Init_Title();
void Init_Menu();
void Init_World_Select();
void Init_Debug();

void Draw_Debug();

void Bind_Current_Document();

void Toggle_Options_Menu();
void Toggle_VSync();
void Toggle_AO();
void Toggle_Wireframe();
void Change_Render_Distance();

void UI::Init() {
    Interface::Init();
    inventory.Init();
    chat.Init();

    Init_Title();
    Init_Menu();
    Init_World_Select();
    Init_Debug();
}

void UI::Draw() {
    if (Wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (ShowTitle || ShowGameMenu) {
        Interface::Mouse_Handler(player.LastMousePos.x, player.LastMousePos.y);

        if (ShowTitle) {
            if (ShowOptions) {
                Interface::Draw_Document("titleOptions");
            }
            else {
                Interface::Draw_Document("title");
            }
        }

        else {
            if (ShowOptions) {
                Interface::Draw_Document("options");
            }
            else {
                Interface::Draw_Document("gameMenu");
            }
        }
    }

    else {
        chat.Update();
        inventory.Draw();

        if (ShowDebug) {
            Draw_Debug();
        }
    }

    if (Wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
}

void UI::Click(int action, int button) {
    Bind_Current_Document();

    if (ShowInventory && !ShowGameMenu) {
        inventory.Click_Handler(button, action);
    }
    else {
        Interface::Click(button, action);
    }

    Interface::Set_Document("");

    if (!GamePaused) {
        player.Click_Handler(button, action);
    }
}

void UI::Mouse_Handler(double x, double y) {
    Bind_Current_Document();
    Interface::Mouse_Handler(x, SCREEN_HEIGHT - y);
    Interface::Set_Document("");

    if (chat.Focused && !chat.FocusToggled) {
        chat.Mouse_Handler(x, y);
    }

    if (!GamePaused && !chat.Focused) {
        player.Mouse_Handler(x, y);
    }
}

void UI::Key_Handler(int key, int action) {
    if (action == GLFW_PRESS) {
        if (Interface::HoveringType == "textBox") {
            static_cast<TextBox*>(Interface::HoveringElement)->Key_Handler(key);
        }

        switch (key) {
            case GLFW_KEY_ESCAPE:
                if (ShowInventory) {
                    inventory.Is_Open = false;
                    Toggle_Inventory();
                }
                else {
                    Toggle_Game_Menu();
                }
                break;

            case GLFW_KEY_U:
                Toggle_Debug();
                break;

            case GLFW_KEY_TAB:
                Toggle_Inventory();
                break;
        }
    }
}

void UI::Text_Handler(unsigned int codepoint) {
    if (chat.Focused && !chat.FocusToggled) {
        chat.Input(codepoint);
    }
    else {
        if (Interface::HoveringType == "textBox") {
            static_cast<TextBox*>(Interface::HoveringElement)->Input(codepoint);
        }
    }
}

void UI::Toggle_Title() {
    ShowGameMenu = false;
    ShowOptions = false;
    ShowInventory = false;
    ShowDebug = false;

    inventory.Is_Open = false;

    ShowTitle = !ShowTitle;
    GamePaused = ShowTitle;
    Toggle_Mouse(ShowTitle);
}

void UI::Toggle_Game_Menu() {
    if (!ShowTitle) {
        ShowGameMenu = !ShowGameMenu;
        ShowOptions = false;

        if (!ShowInventory) {
            Toggle_Mouse(ShowGameMenu);
        }
    }
}

void UI::Toggle_Debug() {
    if (!ShowTitle) {
        ShowDebug = !ShowDebug;
    }
}

void UI::Toggle_Inventory() {
    if (!ShowTitle) {
        ShowInventory = !ShowInventory;
        Toggle_Mouse(ShowInventory);
    }
}

void UI::Toggle_Mouse(bool enable) {
    MouseEnabled = enable;
    glfwSetInputMode(Window, GLFW_CURSOR, enable ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}

void Bind_Current_Document() {
    std::string name;

    if (ShowTitle) {
        if (ShowOptions) {
            name = "titleOptions";
        }
        else {
            name = "title";
        }
    }
    else if (ShowGameMenu) {
        if (ShowOptions) {
            name = "options";
        }
        else {
            name = "gameMenu";
        }
    }

    Interface::Set_Document(name);
}

void Init_Title() {
    glm::vec2 buttonSize = Scale(200, 40);

    glm::vec4 bgDims(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glm::vec3 logoDims(Scale(0, 700), 0.5f);

    glm::vec4 singleButtonDims(Scale(620, 500), buttonSize);
    glm::vec4 multiButtonDims(Scale(620, 430), buttonSize);
    glm::vec4 optionButtonDims(Scale(620, 300), buttonSize);
    glm::vec4 exitButtonDims(Scale(620, 200), buttonSize);

    glm::vec4 vsyncButtonDims(Scale(420, 500), buttonSize);
    glm::vec4 aoButtonDims(Scale(420, 400), buttonSize);
    glm::vec4 wireframeButtonDims(Scale(780, 500), buttonSize);
    glm::vec4 renderDistSliderDims(Scale(620, 700), buttonSize);
    glm::vec4 backButtonDims(Scale(620, 200), buttonSize);

    glm::vec4 textBoxDims(Scale(620, 100), buttonSize);

    glm::vec3 renderDistSliderRange(1, 10, RENDER_DISTANCE);

    Interface::Set_Document("title");
        Interface::Add_Background("titleBg", bgDims);
        Interface::Get_Background("titleBg")->Color = glm::vec3(0.2f);
        Interface::Get_Background("titleBg")->Opacity = 1.0f;

        Interface::Add_Image("titleLogo", "logo.png", 3, logoDims);
        Interface::Get_Image("titleLogo")->Center();

        Interface::Add_Button("titleSingle", "Single Player", singleButtonDims, UI::Toggle_Title);
        Interface::Add_Button("titleMulti", "Multi Player", multiButtonDims, UI::Toggle_Title);
        Interface::Add_Button("options", "Options", optionButtonDims, Toggle_Options_Menu);
        Interface::Add_Button("titleExit", "Quit", exitButtonDims, Exit);
        Interface::Add_Text_Box("test", textBoxDims);
    Interface::Set_Document("");

    Interface::Set_Document("titleOptions");
        Interface::Add_Background("menuBg", bgDims);
        Interface::Get_Background("menuBg")->Color = glm::vec3(0.2f);
        Interface::Get_Background("menuBg")->Opacity = 1.0f;

        Interface::Add_Button("option_vsync", "V-Sync: " + BoolStrings[VSYNC], vsyncButtonDims, Toggle_VSync);
        Interface::Add_Button("option_wireframe", "Wireframe: " + BoolStrings[Wireframe], wireframeButtonDims, Toggle_Wireframe);
        Interface::Add_Slider("option_renderDistance", "Render Distance: " + std::to_string(RENDER_DISTANCE), renderDistSliderDims, renderDistSliderRange, Change_Render_Distance);
        Interface::Add_Button("option_ao", "Ambient Occlusion: " + BoolStrings[AMBIENT_OCCLUSION], aoButtonDims, Toggle_AO);
        Interface::Add_Button("option_back", "Back", backButtonDims, Toggle_Options_Menu);
    Interface::Set_Document("");
}

void Init_Menu() {
    glm::vec2 buttonSize(Scale(200, 40));

    glm::vec4 bgDims(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    glm::vec4 optionButtonDims(Scale(620, 500), buttonSize);
    glm::vec4 exitButtonDims(Scale(620, 200), buttonSize);
    glm::vec4 vsyncButtonDims(Scale(420, 500), buttonSize);
    glm::vec4 aoButtonDims(Scale(420, 400), buttonSize);
    glm::vec4 wireframeButtonDims(Scale(780, 500), buttonSize);
    glm::vec4 renderDistSliderDims(Scale(620, 700), buttonSize);
    glm::vec4 backButtonDims(Scale(620, 200), buttonSize);

    glm::vec3 renderDistSliderRange(1, 10, RENDER_DISTANCE);

    Interface::Set_Document("gameMenu");

    Interface::Add_Background("menuBg", bgDims);
    Interface::Add_Button("options", "Options", optionButtonDims, Toggle_Options_Menu);
    Interface::Add_Button("exit", "Quit to Menu", exitButtonDims, UI::Toggle_Title);

    Interface::Set_Document("options");

    Interface::Add_Background("menuBg", bgDims);
    Interface::Add_Button("option_vsync", "V-Sync: " + BoolStrings[VSYNC], vsyncButtonDims, Toggle_VSync);
    Interface::Add_Button("option_wireframe", "Wireframe: " + BoolStrings[Wireframe], wireframeButtonDims, Toggle_Wireframe);
    Interface::Add_Slider("option_renderDistance", "Render Distance: " + std::to_string(RENDER_DISTANCE), renderDistSliderDims, renderDistSliderRange, Change_Render_Distance);
    Interface::Add_Button("option_ao", "Ambient Occlusion: " + BoolStrings[AMBIENT_OCCLUSION], aoButtonDims, Toggle_AO);
    Interface::Add_Button("option_back", "Back", backButtonDims, Toggle_Options_Menu);

    Interface::Set_Document("");
}

void Init_World_Select() {
    Interface::Set_Document("worlds");

    Interface::Set_Document("");
}

void Init_Debug() {
    lastUIUpdate = glfwGetTime();

    Interface::Set_Document("debug");

    Interface::Add_Text("cpu",         "CPU: 0%",                                  Scale(30, 820));
    Interface::Add_Text("ram",         "RAM: " + System::GetPhysicalMemoryUsage(), Scale(30, 790));
    Interface::Add_Text("chunkQueue",  "Chunks Loaded: ",                          Scale(30, 760));

    Interface::Set_Document("");
}

int Get_Loaded() {
    int total = 0;

    for (auto const &chunk : ChunkMap) {
        total += chunk.second->Meshed;
    }

    return total;
}

void Draw_Debug() {
    CPU.push_back(int(System::GetCPUUsage()));

    if (CPU.size() > AVG_UPDATE_RANGE) {
        CPU.pop_front();
    }

    Interface::Set_Document("debug");

    if (LastFrame - lastUIUpdate >= UI_UPDATE_FREQUENCY) {
        lastUIUpdate = LastFrame;

        int cpu_sum = 0;

        for (int const &time : CPU) {
            cpu_sum += time;
        }

        Interface::Get_Text_Element("cpu")->Set_Text(
            "CPU: " + std::to_string(int(cpu_sum / AVG_UPDATE_RANGE)) + "%"
        );
        Interface::Get_Text_Element("ram")->Set_Text(
            "RAM: " + System::GetPhysicalMemoryUsage()
        );
    }

    Interface::Get_Text_Element("chunkQueue")->Set_Text(
        "Chunks Queued: " + std::to_string(static_cast<int>(ChunkMap.size()) - Get_Loaded())
    );

    Interface::Set_Document("");
    Interface::Draw_Document("debug");
}

void Toggle_Options_Menu() {
    ShowOptions = !ShowOptions;
}

void Toggle_VSync() {
    VSYNC = !VSYNC;

    Bind_Current_Document();
    Interface::Get_Button("option_vsync")->Text.Set_Text(
        "V-Sync: " + BoolStrings[VSYNC]
    );
    Interface::Set_Document("");

    glfwSwapInterval(VSYNC);
    Write_Config();
}

void Toggle_AO() {
    AMBIENT_OCCLUSION = !AMBIENT_OCCLUSION;

    Bind_Current_Document();
    Interface::Get_Button("option_ao")->Text.Set_Text(
        "Ambient Occlusion: " + BoolStrings[AMBIENT_OCCLUSION]
    );
    Interface::Set_Document("");

    Write_Config();
    player.Queue_Chunks(true);
}

void Toggle_Wireframe() {
    ToggleWireframe = true;

    Bind_Current_Document();
    Interface::Get_Button("option_wireframe")->Text.Set_Text(
        "Wireframe: " + BoolStrings[!Wireframe]
    );
    Interface::Set_Document("");
}

void Change_Render_Distance() {
    Bind_Current_Document();
    Slider* slider = Interface::Get_Slider("option_renderDistance");
    Interface::Set_Document("");

    int value = static_cast<int>(std::ceil(slider->Value));

    if (value != RENDER_DISTANCE) {
        RENDER_DISTANCE = value;
        Write_Config();

        player.Queue_Chunks();
    }
}
