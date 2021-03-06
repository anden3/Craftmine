#include "UI.h"

#include "Chat.h"
#include "main.h"
#include "Chunk.h"
#include "Blocks.h"
#include "Player.h"
#include "System.h"
#include "Worlds.h"
#include "Network.h"
#include "Interface.h"
#include "Inventory.h"

#include <fstream>
#include <numeric>
#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>

const int AVG_UPDATE_RANGE = 10;
const double UI_UPDATE_FREQUENCY = 1.0;

const std::string FONT = "Roboto";

static double lastUIUpdate;
static std::deque<int> CPU;

bool UI::ShowDebug        = false;
bool UI::ShowTitle        = true;
bool UI::ShowWorlds       = false;
bool UI::ShowOptions      = false;
bool UI::ShowServers      = false;
bool UI::ShowGameMenu     = false;
bool UI::ShowInventory    = false;
bool UI::ShowVideoOptions = false;

int UI::MouseX = 0;
int UI::MouseY = 0;

std::string UI::CustomDocument = "";

const std::string BoolStrings[2] = {"False", "True"};

void Init_Menu();
void Init_Debug();
void Init_Title();
void Init_Options();
void Init_World_Select();
void Init_Server_Screen();

void Create_World_List();
void Create_Server_List();

void Toggle_Debug();
void Toggle_Inventory();
void Toggle_Game_Menu();

void Toggle_Title(void* caller);
void Toggle_World_Screen(void* caller);
void Toggle_Server_Screen(void* caller);
void Toggle_Options_Menu(void* caller);
void Toggle_Video_Options(void* caller);

void Draw_Debug();

void Bind_Current_Document();

void Toggle_AO(void* caller);
void Toggle_VSync(void* caller);
void Toggle_Wireframe(void* caller);

std::pair<bool, float> Get_Slider_Value(void* slider, int &storage) {
    float value = std::round(static_cast<Slider*>(slider)->Value);

    if (value == storage) {
        return {false, 0.0f};
    }

    storage = static_cast<int>(value);
    Write_Config();

    return {true, value};
}

void Change_FOV(void* caller);
void Change_Mipmap_Level(void* caller);
void Change_Render_Distance(void* caller);
void Change_Anisotropic_Filtering(void* caller);

void Create_World(void* caller);
void Load_World(void* caller);
void Delete_World(void* caller);

void Add_Server(void* caller);
void Load_Server(void* caller);
void Delete_Server(void* caller);

void UI::Init() {
    Interface::Init();
    Inventory::Init();
    Chat::Init();

    Init_Title();
    Init_Options();
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
            Interface::Draw_Document("titleBg");
        }
        else {
            Interface::Draw_Document("menuBg");
        }

        if (ShowOptions) {
            if (ShowVideoOptions) {
                Interface::Draw_Document("videoOptions");
            }
            else {
                Interface::Draw_Document("options");
            }
        }

        else if (ShowGameMenu) {
            Interface::Draw_Document("gameMenu");
        }

        else if (ShowTitle) {
            if (ShowServers) {
                Interface::Draw_Document("servers");
            }
            else if (ShowWorlds) {
                Interface::Draw_Document("worlds");
            }
            else {
                Interface::Draw_Document("title");
            }
        }
    }

    else {
        Chat::Update();

        if (CustomDocument != "") {
            Interface::Draw_Document("inventory");
            Interface::Draw_Document(CustomDocument);
            Interface::Draw_Document("mouseStack");
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

void UI::Mouse_Handler(int x, int y) {
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
                    player.LookingBlockType->CloseFunction();
                }

                else if (ShowInventory) {
                    Inventory::Is_Open = false;
                    Toggle_Inventory();
                }
                else if (ShowVideoOptions) {
                    Toggle_Video_Options(nullptr);
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

    if (UI::ShowOptions) {
        if (UI::ShowVideoOptions) { name = "videoOptions"; }
        else { name = "options"; }
    }

    else if (UI::ShowTitle) {
        if (UI::ShowServers) { name = "servers"; }
        else if (UI::ShowWorlds) { name = "worlds"; }
        else { name = "title"; }
    }
    else if (UI::ShowGameMenu) { name = "gameMenu"; }
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
    glm::vec4 backButtonDims(Scale(620, 200), buttonSize);

    Interface::Set_Document("titleBg");
        Interface::Add_Background("bg", bgDims);
        Interface::Get_Background("bg")->Color = glm::vec3(0.2f);
        Interface::Get_Background("bg")->Opacity = 1.0f;
    Interface::Set_Document("");

    Interface::Set_Document("title");
        Interface::Add_Image("logo", "logo.png", 3, logoDims);
        Interface::Get_Image("logo")->Center();

        Interface::Add_Button("single", "Single Player", singleButtonDims, Toggle_World_Screen);
        Interface::Add_Button("multi", "Multi Player", multiButtonDims, Toggle_Server_Screen);
        Interface::Add_Button("options", "Options", optionButtonDims, Toggle_Options_Menu);
        Interface::Add_Button("exit", "Quit", exitButtonDims, Exit);
    Interface::Set_Document("");
}

void Init_Options() {
    glm::vec2 buttonSize(Scale(200, 40));

    glm::vec4 vsyncButtonDims(Scale(400, 500), buttonSize);
    glm::vec4 aoButtonDims(Scale(620, 500), buttonSize);
    glm::vec4 wireframeButtonDims(Scale(840, 500), buttonSize);

    glm::vec4 videoOptionsDims(Scale(400, 500), buttonSize);
    glm::vec4 backButtonDims(Scale(620, 200), buttonSize);

    glm::vec4 afDims(Scale(400, 700), buttonSize);
    glm::vec4 renderDistDims(Scale(620, 700), buttonSize);
    glm::vec4 fovDims(Scale(840, 700), buttonSize);
    glm::vec4 mipmapDims(Scale(400, 600), buttonSize);

    glm::vec3 renderDistRange(1, 20, RENDER_DISTANCE);
    glm::vec3 afRange(1, 16, ANISOTROPIC_FILTERING);
    glm::vec3 mipmapRange(0, 4, MIPMAP_LEVEL);
    glm::vec3 fovRange(10, 180, FOV);

    Interface::Set_Document("options");
        Interface::Add_Button("videoOptions", "Video Options", videoOptionsDims, Toggle_Video_Options);
        Interface::Add_Button("back", "Back", backButtonDims, Toggle_Options_Menu);
    Interface::Set_Document("");

    Interface::Set_Document("videoOptions");
        Interface::Add_Button("vsync", "V-Sync: " + BoolStrings[VSYNC], vsyncButtonDims, Toggle_VSync);
        Interface::Add_Button("wireframe", "Wireframe: " + BoolStrings[Wireframe], wireframeButtonDims, Toggle_Wireframe);
        Interface::Add_Button("ao", "Ambient Occlusion: " + BoolStrings[AMBIENT_OCCLUSION], aoButtonDims, Toggle_AO);
        Interface::Add_Button("back", "Back", backButtonDims, Toggle_Video_Options);

        Interface::Add_Slider(
            "af", "Anisotropic Filtering: " + std::to_string(ANISOTROPIC_FILTERING),
            afDims, afRange, Change_Anisotropic_Filtering
        );
        Interface::Add_Slider(
            "renderDistance", "Render Distance: " + std::to_string(RENDER_DISTANCE),
            renderDistDims, renderDistRange, Change_Render_Distance
        );
        Interface::Add_Slider("fov", "FOV: " + std::to_string(FOV), fovDims, fovRange, Change_FOV);
        Interface::Add_Slider(
            "mipmap", "Mipmapping Level: " + std::to_string(MIPMAP_LEVEL),
            mipmapDims, mipmapRange, Change_Mipmap_Level
        );
    Interface::Set_Document("");
}

void Init_Menu() {
    glm::vec2 buttonSize(Scale(200, 40));
    glm::vec4 bgDims(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glm::vec4 optionButtonDims(Scale(620, 500), buttonSize);
    glm::vec4 exitButtonDims(Scale(620, 200), buttonSize);

    Interface::Set_Document("menuBg");
        Interface::Add_Background("menuBg", bgDims);
    Interface::Set_Document("");

    Interface::Set_Document("gameMenu");
        Interface::Add_Button("options", "Options", optionButtonDims, Toggle_Options_Menu);
        Interface::Add_Button("exit", "Quit to Menu", exitButtonDims, Toggle_Title);
    Interface::Set_Document("");
}

void Init_World_Select() {
    glm::vec4 bgDims(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glm::vec4 newNameDims(Scale(100, 800), Scale(200, 35));
    glm::vec4 newSeedDims(Scale(100, 700), Scale(200, 35));
    glm::vec4 newWorldDims(Scale(100, 600), Scale(200, 40));
    glm::vec4 backButtonDims(Scale(620, 200), Scale(200, 40));

    Interface::Set_Document("worlds");
        Interface::Add_Text("newNameLabel", "Name:", Scale(100, 840));
        Interface::Add_Text_Box("newName", newNameDims);

        Interface::Add_Text("newSeedLabel", "Seed:", Scale(100, 740));
        Interface::Add_Text_Box("newSeed", newSeedDims);

        Interface::Add_Button("create", "Create New World", newWorldDims, Create_World);
        Interface::Add_Button("back", "Back", backButtonDims, Toggle_World_Screen);
    Interface::Set_Document("");

    Create_World_List();
}

void Create_World_List() {
    static float worldStartY = 800;
    static float worldSpacing = 100;

    static float worldXPos = Scale_X(520);
    static glm::vec2 worldSize(Scale(350, 40));

    static float deleteButtonXPos = Scale_X(880);
    static glm::vec2 deleteButtonSize(Scale(40, 40));

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
    glm::vec4 nameDims(Scale(100, 800), Scale(200, 35));
    glm::vec4 serverNameDims(Scale(100, 700), Scale(200, 35));
    glm::vec4 ipDims(Scale(100, 600), Scale(200, 35));
    glm::vec4 addServerDims(Scale(100, 500), Scale(200, 40));
    glm::vec4 backDims(Scale(620, 200), Scale(200, 40));

    Interface::Set_Document("servers");
        Interface::Add_Text("nameLabel", "User Name", Scale(100, 840));
        Interface::Add_Text_Box("name", nameDims);

        Interface::Add_Text("serverNameLabel", "Server Name", Scale(100, 740));
        Interface::Add_Text_Box("serverName", serverNameDims);

        Interface::Add_Text("ipLabel", "Server Address", Scale(100, 640));
        Interface::Add_Text_Box("ip", ipDims);

        Interface::Add_Button("addServer", "Add", addServerDims, Add_Server);

        Interface::Add_Text("errMsg", "", Scale(50, 570));
        Interface::Get_Text_Element("errMsg")->Center(Scale(50, 570), Scale_X(300), {true, false});

        Interface::Add_Button("back", "Back", backDims, Toggle_Server_Screen);
    Interface::Set_Document("");

    Create_Server_List();
}

void Create_Server_List() {
    static float serverStartY = 800;
    static float serverSpacing = 100;

    static float serverXPos = Scale_X(520);
    static glm::vec2 serverSize(Scale(350, 40));

    static float deleteButtonXPos = Scale_X(880);
    static glm::vec2 deleteButtonSize(Scale(40, 40));

    std::fstream serverFile("servers.json");
    bool fileExists = serverFile.is_open();
    serverFile.close();

    if (!fileExists) {
        return;
    }

    nlohmann::json json;
    serverFile.open("servers.json", std::ifstream::in);
    json << serverFile;
    serverFile.close();

    Interface::Set_Document("servers");
        for (auto it = json.begin(); it != json.end(); ++it) {
            std::string serverName = it.key();

            Interface::Delete_Button(serverName);
            Interface::Delete_Button("remove_" + serverName);

            Interface::Add_Button(
                serverName, serverName, glm::vec4(serverXPos, Scale_Y(serverStartY), serverSize), Load_Server
            );

            Interface::Add_Button(
                "remove_" + serverName, "X", glm::vec4(deleteButtonXPos, Scale_Y(serverStartY), deleteButtonSize), Delete_Server
            );

            serverStartY -= serverSpacing;
        }
    Interface::Set_Document("");
}

void Init_Debug() {
    lastUIUpdate = glfwGetTime();

    Interface::Set_Document("debug");

    std::string ramUsage = System::GetPhysicalMemoryUsage();

    Interface::Add_Text("cpu",         "CPU: 0%",           Scale(30, 820));
    Interface::Add_Text("ram",         "RAM: " + ramUsage,  Scale(30, 790));
    Interface::Add_Text("chunkQueue",  "Chunks Loaded: ",   Scale(30, 760));
    Interface::Add_Text("vertQueue",   "Vertices Loaded: ", Scale(30, 730));

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
    if (!UI::ShowTitle && UI::CustomDocument == "") {
        UI::ShowInventory = !UI::ShowInventory;
        UI::Toggle_Mouse(UI::ShowInventory);
    }
}

void Toggle_Debug() {
    if (!UI::ShowTitle) {
        UI::ShowDebug = !UI::ShowDebug;
    }
}

void Toggle_Options_Menu(void* caller) {
    UI::ShowOptions = !UI::ShowOptions;
}

void Toggle_Video_Options(void* caller) {
    UI::ShowVideoOptions = !UI::ShowVideoOptions;
}

std::tuple<int, int> Get_Loaded() {
    int total = 0;
    int vertices = 0;

    for (auto const &chunk : ChunkMap) {
        total += chunk.second->Meshed;

        if (chunk.second->Visible) {
            vertices += chunk.second->buffer.Vertices;
        }
    }

    return {total, vertices};
}

void Draw_Debug() {
    CPU.push_back(static_cast<int>(System::GetCPUUsage()));

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
            "CPU: " + std::to_string(static_cast<int>(cpu_sum / AVG_UPDATE_RANGE)) + "%"
        );
        Interface::Get_Text_Element("ram")->Set_Text(
            "RAM: " + System::GetPhysicalMemoryUsage()
        );
    }

    int loadedChunks, loadedVertices;
    std::tie(loadedChunks, loadedVertices) = Get_Loaded();

    Interface::Get_Text_Element("chunkQueue")->Set_Text(
        "Chunks Queued: " + std::to_string(static_cast<int>(ChunkMap.size()) - loadedChunks)
    );

    Interface::Get_Text_Element("vertQueue")->Set_Text("Vertices Loaded: " + std::to_string(loadedVertices));

    Interface::Set_Document("");
    Interface::Draw_Document("debug");
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
    auto result = Get_Slider_Value(caller, RENDER_DISTANCE);

    if (result.first && !UI::ShowTitle) {
        player.Queue_Chunks();
    }
}

void Change_Anisotropic_Filtering(void* caller) {
    auto result = Get_Slider_Value(caller, ANISOTROPIC_FILTERING);

    if (!result.first) {
        return;
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, Load_Array_Texture("atlas.png", {16, 32}, MIPMAP_LEVEL, result.second));
}

void Change_FOV(void* caller) {
    auto result = Get_Slider_Value(caller, FOV);

    if (!result.first) {
        return;
    }

    glm::mat4 projection = glm::perspective(
        glm::radians(static_cast<float>(FOV)),
        static_cast<float>(SCREEN_WIDTH) / SCREEN_HEIGHT,
        Z_NEAR_LIMIT, Z_FAR_LIMIT
    );

    UBO.Upload(1, projection);
}

void Change_Mipmap_Level(void* caller) {
    auto result = Get_Slider_Value(caller, MIPMAP_LEVEL);

    if (!result.first) {
        return;
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, Load_Array_Texture("atlas.png", {16, 32}, MIPMAP_LEVEL, static_cast<float>(ANISOTROPIC_FILTERING)));
}

void Create_World(void* caller) {
    Interface::Set_Document("worlds");
        TextBox* name = Interface::Get_Text_Box("newName");
        TextBox* seed = Interface::Get_Text_Box("newSeed");
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

void Add_Server(void* caller) {
    Interface::Set_Document("servers");
        TextElement* errMsg     = Interface::Get_Text_Element("errMsg");
        TextBox*     serverName = Interface::Get_Text_Box("serverName");
        TextBox*     username   = Interface::Get_Text_Box("name");
        TextBox*     ipEl       = Interface::Get_Text_Box("ip");
    Interface::Set_Document("");

    std::string host = ipEl->Text;

    if (username->Text == "") {
        errMsg->Set_Text("&cError! &fPlease input a user name.");
        return;
    }

    if (serverName->Text == "") {
        errMsg->Set_Text("&cError! &fPlease input a server name.");
        return;
    }

    if (host == "") {
        errMsg->Set_Text("&cError! &fPlease input an IP address.");
        return;
    }

    if (std::count(host.begin(), host.end(), '.') != 3) {
        errMsg->Set_Text("&cError! &fInvalid IP address.");
        return;
    }

    std::string ip;
    unsigned short port;

    if (host.find(':') == std::string::npos || host.find(':') == host.length() - 1) {
        if (ip.find(':') != std::string::npos) {
            ip = host.substr(0, host.length() - 1);
        }
        else {
            ip = host;
        }
    }
    else {
        try {
            port = static_cast<unsigned short>(std::stoi(host.substr(host.find(':') + 1)));
            ip = host.substr(0, host.find(':'));
        }
        catch (...) {
            errMsg->Set_Text("&cError! &fInvalid port.");
            return;
        }
    }

    auto ipParts = Split(ip, '.');

    if (ipParts.size() < 4) {
        errMsg->Set_Text("&cError! &fMissing IP value.");
        return;
    }
    else if (ipParts.size() > 4) {
        errMsg->Set_Text("&cError! &Too many IP values.");
        return;
    }

    for (std::string const &part : ipParts) {
        if (part.length() > 1 && part.front() == '0') {
            errMsg->Set_Text("&cError! &fPlease remove leading zeroes from IP values.");
            return;
        }

        try {
            int partNum = std::stoi(part);

            if (partNum > 255) {
                errMsg->Set_Text("&cError! &fIP value out of range. Value &6" + part + " &fis out of range (&60 &f- &6255&f).");
                return;
            }
        }
        catch (const std::invalid_argument) {
            errMsg->Set_Text("&cError! &fNon-numeric characters in IP.");
            return;
        }
    }

    nlohmann::json servers;
    std::fstream serverFile("servers.json");

    bool fileExists = serverFile.is_open();

    if (fileExists) {
        serverFile.close();

        serverFile.open("servers.json", std::ifstream::in);
        servers << serverFile;
        serverFile.close();
    }
    else {
        serverFile.open("servers.json", std::ifstream::out | std::ifstream::trunc);
    }

    servers[serverName->Text] = {{"username", username->Text}, {"ip", host}};

    serverName->Clear();
    username->Clear();
    ipEl->Clear();

    if (fileExists) {
        serverFile.open("servers.json", std::ifstream::out | std::ifstream::trunc);
    }

    servers >> serverFile;
    serverFile.close();

    Create_Server_List();
}

void Load_Server(void* caller) {
    nlohmann::json json;
    std::ifstream file("servers.json");
    json << file;
    file.close();

    std::string serverName = static_cast<Button*>(caller)->Name;
    std::string serverIP = json[serverName]["ip"];

    PLAYER_NAME = json[serverName]["username"];

    if (Network::Connect(PLAYER_NAME, serverIP)) {
        Multiplayer = true;
    }
    else {
        Interface::Set_Document("servers");
        Interface::Get_Text_Element("errMsg")->Set_Text("&cError! &fCould not connect to server!");
        Interface::Set_Document("");
    }
}

void Delete_Server(void* caller) {
    // Remove remove_ prefix.
    std::string serverName = static_cast<Button*>(caller)->Name.substr(7);

    Interface::Set_Document("servers");
        Interface::Delete_Button(serverName);
        Interface::Delete_Button("remove_" + serverName);
    Interface::Set_Document("");

    nlohmann::json json;
    std::ifstream file("servers.json");
    json << file;
    file.close();

    json.erase(serverName);

    std::ofstream outFile("servers.json");
    json >> outFile;
    outFile.close();

    Create_Server_List();
}

#ifdef __clang__
    #pragma clang diagnostic pop
#elif _MSC_VER
    #pragma warning(pop)
#endif
