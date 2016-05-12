#pragma once

#include "Player.h"

#include "Text.h"
#include "Button.h"
#include "Slider.h"

extern const bool Windows;

extern int VSync;
extern bool Wireframe;
extern bool ToggleWireframe;
extern bool MouseEnabled;

extern int RenderDistance;

extern double DeltaTime;
extern double LastFrame;

extern GLFWwindow* Window;
extern Player player;
extern Chat chat;

namespace UI {
    void Init();
    void Draw();
    void Clean();
    
    void Click(double mouseX, double mouseY, int action, int button);
    
    void Toggle_Menu();
    void Toggle_Debug();
    void Toggle_Inventory();
};

std::string Format_Vector(glm::vec3 vector);