#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Text.h"
#include "Player.h"

extern const bool Windows;

extern bool VSYNC;
extern bool Wireframe;
extern bool ToggleWireframe;

extern double deltaTime;
extern double lastFrame;

extern GLFWwindow* Window;
extern Player player;

namespace UI {
    void Init();
    void Draw();
    void Clean();
    
    void Toggle_Menu();
    void Toggle_Debug();
};

std::string Format_Vector(glm::vec3 vector);