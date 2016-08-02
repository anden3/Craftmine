#include "UI.h"

#include "Chat.h"
#include "main.h"
#include "Chunk.h"
#include "Player.h"
#include "System.h"
#include "Worlds.h"
#include "Network.h"
#include "Interface.h"
#include "Inventory.h"

#include <numeric>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

const int AVG_UPDATE_RANGE = 10;
const double UI_UPDATE_FREQUENCY = 1.0;

const std::string FONT = "Roboto";

static double lastUIUpdate;
static std::deque<int> CPU;

bool UI::ShowDebug     = false;
bool UI::ShowTitle     = true;
bool UI::ShowWorlds    = false;
bool UI::ShowOptions   = false;
bool UI::ShowServers   = false;
bool UI::ShowGameMenu  = false;
bool UI::ShowInventory = false;

double UI::MouseX = 0.0;
double UI::MouseY = 0.0;

std::string UI::CustomDocument = "";

const std::string BoolStrings[2] = {"False", "True"};

void Init_Menu();
void Init_Debug();
void Init_Title();
void Init_World_Select();
void Init_Server_Screen();

void Create_World_List();

void Toggle_Debug();
void Toggle_Inventory();
void Toggle_Game_Menu();

void Toggle_Title(void* caller);
void Toggle_World_Screen(void* caller);
void Toggle_Server_Screen(void* caller);

void Draw_Debug();

void Bind_Current_Document();

void Toggle_AO(void* caller);
void Toggle_VSync(void* caller);
void Toggle_Wireframe(void* caller);
void Toggle_Options_Menu(void* caller);
void Change_Render_Distance(void* caller);

void Create_World(void* caller);
void Load_World(void* caller);
void Delete_World(void* caller);

void Connect_To_Server(void* caller);

void UI::Init() {
    Interface::Init();
    Inventory::Init();
    Chat::Init();

    Init_Title();
    Init_Menu();
    Init_World_Select();
    Init_Server_Screen();
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
            else if (ShowServers) {
                Interface::Draw_Document("servers");
            }
            else if (ShowWorlds) {
                Interface::Draw_Document("worlds");
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
        Chat::Update();

        if (CustomDocument != "") {
            Interface::Draw_Document(CustomDocument);
        }
        else {
            Inventory::Draw();

            if (ShowDebug) {
                Draw_Debug();
            }
        }
    }

    if (Wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
}

void UI::Click(int action, int button) {
    Bind_Current_Document();
    Interface::Click(button, action);
    Interface::Set_Document("");

    if (!GamePaused) {
        player.Click_Handler(button, action);
    }
}

void UI::Load_World(int seed) {
    Worlds::Load_World(seed);

    ShowTitle = false;
    ShowWorlds = false;
    GamePaused = false;

    UI::Toggle_Mouse(false);
}

void UI::Mouse_Handler(double x, double y) {
    MouseX = x;
    MouseY = y;

    Bind_Current_Document();
    Interface::Mouse_Handler(x, SCREEN_HEIGHT - y);
    Interface::Set_Document("");

    if (Chat::Focused && !Chat::FocusToggled) {
        Chat::Mouse_Handler(x, y);
    }

    if (!GamePaused && !Chat::Focused) {
        player.Mouse_Handler(x, y);
    }
}

void UI::Key_Handler(int key, int action) {
    if (action != GLFW_RELEASE) {
        if (Interface::HoveringType == "textBox") {
            static_cast<TextBox*>(Interface::HoveringElement)->Key_Handler(key);
        }
    }

    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                if (CustomDocument != "") {
                    CustomDocument = "";
                    UI::Toggle_Mouse(false);
                }

                else if (ShowInventory) {
                    Inventory::Is_Open = false;
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
    if (Chat::Focused && !Chat::FocusToggled) {
        Chat::Input(codepoint);
    }
    else {
        if (Interface::HoveringType == "textBox") {
            static_cast<TextBox*>(Interface::HoveringElement)->Input(codepoint);
        }
    }
}

void UI::Toggle_Mouse(bool enable) {
    MouseEnabled = enable;
    glfwSetInputMode(Window, GLFW_CURSOR, enable ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}

void Bind_Current_Document() {
    std::string name;

    if (UI::CustomDocument != "") {
        name = UI::CustomDocument;
    }

    else if (UI::ShowTitle) {
        if (UI::ShowOptions) { name = "titleOptions"; }
        else if (UI::ShowServers) { name = "servers"; }
        else if (UI::ShowWorlds) { name = "worlds"; }
        else { name = "title"; }
    }
    else if (UI::ShowGameMenu) {
        if (UI::ShowOptions) { name = "options"; }
        else { name = "gameMenu"; }
    }
    else if (UI::ShowInventory) { name = "inventory"; }

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

    glm::vec3 renderDistSliderRange(1, 10, RENDER_DISTANCE);

    Interface::Set_Document("title");
        Interface::Add_Background("titleBg", bgDims);
        Interface::Get_Background("titleBg")->Color = glm::vec3(0.2f);
        Interface::Get_Background("titleBg")->Opacity = 1.0f;

        Interface::Add_Image("titleLogo", "logo.png", 3, logoDims);
        Interface::Get_Image("titleLogo")->Center();

        Interface::Add_Button("titleSingle", "Single Player", singleButtonDims, Toggle_World_Screen);
        Interface::Add_Button("titleMulti", "Multi Player", multiButtonDims, Toggle_Server_Screen);
        Interface::Add_Button("options", "Options", optionButtonDims, Toggle_Options_Menu);
        Interface::Add_Button("titleExit", "Quit", exitButtonDims, Exit);
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
    Interface::Add_Button("exit", "Quit to Menu", exitButtonDims, Toggle_Title);

    Interface::Set_Document("options");

    Interface::Add_Background("menuBg", bgDims);
    Interface::Add_Button("option_vsync", "V-Sync: " + BoolStrings[VSYNC], vsyncButtonDims, Toggle_VSync);
    Interface::Add_Button("option_wireframe", "Wireframe: " + BoolStrings[Wireframe], wireframeButtonDims, Toggle_Wireframe);
    Interface::Add_Slider(
        "option_renderDistance", "Render Distance: " + std::to_string(RENDER_DISTANCE),
        renderDistSliderDims, renderDistSliderRange, Change_Render_Distance
    );
    Interface::Add_Button("option_ao", "Ambient Occlusion: " + BoolStrings[AMBIENT_OCCLUSION], aoButtonDims, Toggle_AO);
    Interface::Add_Button("option_back", "Back", backButtonDims, Toggle_Options_Menu);

    Interface::Set_Document("");
}

void Init_World_Select() {
    glm::vec4 bgDims(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    glm::vec2 newNameLabelDims(Scale(100, 840));
    glm::vec4 newNameDims(Scale(100, 800), Scale(200, 35));

    glm::vec2 newSeedLabelDims(Scale(100, 740));
    glm::vec4 newSeedDims(Scale(100, 700), Scale(200, 35));

    glm::vec4 newWorldDims(Scale(100, 600), Scale(200, 40));
    glm::vec4 backButtonDims(Scale(620, 200), Scale(200, 40));

    Interface::Set_Document("worlds");
        Interface::Add_Background("worldBg", bgDims);
        Interface::Get_Background("worldBg")->Color = glm::vec3(0.2f);
        Interface::Get_Background("worldBg")->Opacity = 1.0f;

        Interface::Add_Text("worldNewNameLabel", "Name:", newNameLabelDims);
        Interface::Add_Text_Box("worldNewName", newNameDims);

        Interface::Add_Text("worldNewSeedLabel", "Seed:", newSeedLabelDims);
        Interface::Add_Text_Box("worldNewSeed", newSeedDims);

        Interface::Add_Button("worldCreate", "Create New World", newWorldDims, Create_World);
        Interface::Add_Button("worldBack", "Back", backButtonDims, Toggle_World_Screen);
    Interface::Set_Document("");

    Create_World_List();
}

void Create_World_List() {
    float worldStartY = 800;
    float worldSpacing = 100;

    float worldXPos = Scale_X(520);
    glm::vec2 worldSize(Scale(350, 40));

    float deleteButtonXPos = Scale_X(880);
    glm::vec2 deleteButtonSize(Scale(40, 40));

    Interface::Set_Document("worlds");
        for (auto const &world : Worlds::Get_Worlds()) {
            Interface::Delete_Button(world.Name);
            Interface::Delete_Button("remove_" + world.Name);

            Interface::Add_Button(
                world.Name, world.Name, glm::vec4(worldXPos, Scale_Y(worldStartY), worldSize), Load_World
            );

            Interface::Add_Button(
                "remove_" + world.Name, "X", glm::vec4(deleteButtonXPos, Scale_Y(worldStartY), deleteButtonSize), Delete_World
            );

            worldStartY -= worldSpacing;
        }
    Interface::Set_Document("");
}

void Init_Server_Screen() {
    glm::vec4 bgDims(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    glm::vec2 nameLabelPos(Scale(570, 640));
    glm::vec4 nameDims(Scale(570, 600), Scale(300, 35));

    glm::vec2 ipLabelPos(Scale(570, 540));
    glm::vec4 ipDims(Scale(570, 500), Scale(300, 35));

    glm::vec2 errMsgPos(Scale(570, 440));
    float errMsgWidth = Scale_X(300);

    glm::vec4 connectDims(Scale(620, 300), Scale(200, 40));
    glm::vec4 backDims(Scale(620, 200), Scale(200, 40));

    Interface::Set_Document("servers");
        Interface::Add_Background("serverBg", bgDims);
        Interface::Get_Background("serverBg")->Color = glm::vec3(0.2f);
        Interface::Get_Background("serverBg")->Opacity = 1.0f;

        Interface::Add_Text("nameLabel", "User Name", nameLabelPos);
        Interface::Add_Text_Box("name", nameDims);

        Interface::Add_Text("ipLabel", "Server Address", ipLabelPos);
        Interface::Add_Text_Box("ip", ipDims);

        Interface::Add_Text("errMsg", "", errMsgPos);
        Interface::Get_Text_Element("errMsg")->Center(errMsgPos, errMsgWidth, glm::bvec2(true, false));

        Interface::Add_Button("connectToServer", "Connect", connectDims, Connect_To_Server);
        Interface::Add_Button("back", "Back", backDims, Toggle_Server_Screen);
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

#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wunused-parameter"
#elif _MSC_VER
    #pragma warning(push)
    #pragma warning(disable: 4100)
#endif

void Toggle_Title(void* caller) {
    UI::ShowGameMenu = false;
    UI::ShowOptions = false;
    UI::ShowInventory = false;
    UI::ShowDebug = false;

    Inventory::Is_Open = false;

    UI::ShowTitle = !UI::ShowTitle;
    GamePaused = UI::ShowTitle;
    UI::Toggle_Mouse(UI::ShowTitle);

    if (UI::ShowTitle) {
        Worlds::Save_World();
        WORLD_NAME = "";

        if (Multiplayer) {
            Network::Disconnect();
            Network::Update(1000);
            Multiplayer = false;
        }
    }
}

void Toggle_Game_Menu() {
    if (!UI::ShowTitle) {
        UI::ShowGameMenu = !UI::ShowGameMenu;
        UI::ShowOptions = false;

        if (!UI::ShowInventory) {
            UI::Toggle_Mouse(UI::ShowGameMenu);
        }
    }
}

void Toggle_World_Screen(void* caller) {
    UI::ShowWorlds = !UI::ShowWorlds;
}

void Toggle_Server_Screen(void* caller) {
    UI::ShowServers = !UI::ShowServers;
}

void Toggle_Inventory() {
    if (!UI::ShowTitle) {
        UI::ShowInventory = !UI::ShowInventory;
        UI::Toggle_Mouse(UI::ShowInventory);
    }
}

void Toggle_Debug() {
    if (!UI::ShowTitle) {
        UI::ShowDebug = !UI::ShowDebug;
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

void Toggle_Options_Menu(void* caller) {
    UI::ShowOptions = !UI::ShowOptions;
}

void Toggle_VSync(void* caller) {
    VSYNC = !VSYNC;
    static_cast<Button*>(caller)->Text.Set_Text("V-Sync: " + BoolStrings[VSYNC]);

    glfwSwapInterval(VSYNC);
    Write_Config();
}

void Toggle_AO(void* caller) {
    AMBIENT_OCCLUSION = !AMBIENT_OCCLUSION;
    static_cast<Button*>(caller)->Text.Set_Text("Ambient Occlusion: " + BoolStrings[AMBIENT_OCCLUSION]);

    Write_Config();
    player.Queue_Chunks(true);
}

void Toggle_Wireframe(void* caller) {
    if (ToggleWireframe) {
        ToggleWireframe = false;
        static_cast<Button*>(caller)->Text.Set_Text("Wireframe: False");
    }
    else {
        ToggleWireframe = true;
        static_cast<Button*>(caller)->Text.Set_Text("Wireframe: " + BoolStrings[!Wireframe]);
    }
}

void Change_Render_Distance(void* caller) {
    int value = static_cast<int>(std::ceil(static_cast<Slider*>(caller)->Value));

    if (value != RENDER_DISTANCE) {
        RENDER_DISTANCE = value;
        Write_Config();

        player.Queue_Chunks();
    }
}

void Create_World(void* caller) {
    Interface::Set_Document("worlds");
        TextBox* name = Interface::Get_Text_Box("worldNewName");
        TextBox* seed = Interface::Get_Text_Box("worldNewSeed");
    Interface::Set_Document("");

    std::string worldName = name->Text;
    name->Clear();

    std::string seedStr = seed->Text;
    seed->Clear();

    // Check if world already exists.
    if (Worlds::Get_Seed(worldName) != 0) {
        return;
    }

    if (seedStr.length() >= 20) {
        seedStr = seedStr.substr(0, 19);
    }

    int worldSeed;

    try {
        // Prevent integer overflow.
        worldSeed = std::stoll(seedStr) % 2147483647;
    }
    catch (std::invalid_argument) {
        unsigned long long stringSum = std::accumulate(seedStr.begin(), seedStr.end(), static_cast<unsigned long long>(0));
        worldSeed = stringSum % 2147483647;
    }

    Worlds::Create_World(worldName, worldSeed);
    Create_World_List();
}

void Load_World(void* caller) {
    WORLD_NAME = static_cast<Button*>(caller)->Text.Text;
    Worlds::Load_World(Worlds::Get_Seed(WORLD_NAME));

    UI::ShowWorlds = false;
    UI::ShowTitle = false;
    GamePaused = false;
    UI::Toggle_Mouse(false);
}

void Delete_World(void* caller) {
    // Remove remove_ prefix.
    std::string worldName = static_cast<Button*>(caller)->Name.substr(7);

    Interface::Set_Document("worlds");
        Interface::Delete_Button(worldName);
        Interface::Delete_Button("remove_" + worldName);
    Interface::Set_Document("");

    Worlds::Delete_World(worldName);
    Create_World_List();
}

void Connect_To_Server(void* caller) {
    Interface::Set_Document("servers");
        TextElement* errMsg = Interface::Get_Text_Element("errMsg");
        std::string  name   = Interface::Get_Text_Box("name")->Text;
        std::string  ip     = Interface::Get_Text_Box("ip")->Text;
    Interface::Set_Document("");

    PLAYER_NAME = name;

    std::string connectionStatus = Network::Connect(name, ip);
    errMsg->Set_Text(connectionStatus);

    if (connectionStatus == "") {
        Multiplayer = true;
    }
}

#ifdef __clang__
    #pragma clang diagnostic pop
#elif _MSC_VER
    #pragma warning(pop)
#endif