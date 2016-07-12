#pragma once

#define GLM_SWIZZLE
#include <glm/glm.hpp>

#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace UI {
    extern bool ShowTitle;
    extern bool ShowInventory;
    extern bool ShowGameMenu;
    extern bool ShowDebug;
    extern bool ShowOptions;
    extern bool ShowServers;
    extern bool ShowWorlds;

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
