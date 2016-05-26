#pragma once

#include "Player.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

extern const bool Windows;

extern int VSync;
extern bool Wireframe;
extern bool ToggleWireframe;
extern bool MouseEnabled;
extern bool GamePaused;

extern int RenderDistance;

extern double DeltaTime;
extern double LastFrame;

extern GLFWwindow* Window;
extern Player player;
extern Chat chat;

extern void Write_Config();

namespace UI {
    void Init();
    void Draw();
    void Clean();
    
    void Click(double mouseX, double mouseY, int action, int button);
    void Mouse_Handler(double x, double y);
    void Key_Handler(int key, int action);
    
    void Toggle_Title();
    void Toggle_Game_Menu();
    void Toggle_Debug();
    void Toggle_Inventory();
};

std::string Format_Vector(glm::vec3 vector);