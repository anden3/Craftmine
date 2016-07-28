#pragma once

#include <string>

#define GLM_SWIZZLE
#include <glm/glm.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace UI {
    extern bool ShowDebug;
    extern bool ShowTitle;
    extern bool ShowWorlds;
    extern bool ShowOptions;
    extern bool ShowServers;
    extern bool ShowGameMenu;
    extern bool ShowInventory;

    extern std::string CustomDocument;

    void Init();
    void Draw();
    void Clean();

    void Load_World(int seed);

    void Click(int action, int button);
    void Mouse_Handler(double x, double y);
    void Key_Handler(int key, int action);
    void Text_Handler(unsigned int codepoint);

    void Toggle_Mouse(bool enable);
};

std::string Format_Vector(glm::vec3 vector);
