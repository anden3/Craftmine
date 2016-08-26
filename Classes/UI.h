#pragma once

#include <string>

namespace UI {
    extern bool ShowDebug;
    extern bool ShowTitle;
    extern bool ShowWorlds;
    extern bool ShowOptions;
    extern bool ShowServers;
    extern bool ShowGameMenu;
    extern bool ShowInventory;
    extern bool ShowVideoOptions;

    extern int MouseX;
    extern int MouseY;

    extern std::string CustomDocument;

    void Init();
    void Draw();
    void Clean();

    void Load_World(int seed);

    void Click(int action, int button);
    void Mouse_Handler(int x, int y);
    void Key_Handler(int key, int action);
    void Text_Handler(unsigned int codepoint);

    void Toggle_Mouse(bool enable);
};
