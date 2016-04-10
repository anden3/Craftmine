#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Player.h"

#include "Text.h"
#include "Button.h"
#include "Slider.h"
#include "Chat.h"

extern const bool Windows;

extern bool VSync;
extern bool Wireframe;
extern bool ToggleWireframe;

extern int RENDER_DISTANCE;

extern double DeltaTime;
extern double LastFrame;

extern GLFWwindow* Window;
extern Player player;
extern Chat chat;

namespace UI {
    void Init();
    void Draw();
    void Clean();
    
    void Click(double mouseX, double mouseY, int action);
    
    void Toggle_Menu();
    void Toggle_Debug();
};

std::string Format_Vector(glm::vec3 vector);