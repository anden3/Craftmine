#pragma once

#include <string>

#include "Shader.h"

typedef void (Func)(void);

extern int colorLocation;
extern int alphaLocation;

extern Shader* UIShader;
extern Shader* UIBorderShader;

namespace Slider {
    void Add(std::string name, std::string text, Func &function, float x, float y, float w, float min, float max, float start, std::string group = "default");
    void Delete(std::string name);
    void Draw(std::string name);
    
    float Get_Value(std::string name);
    void Set_Text(std::string name, std::string text);
    
    void Move(std::string name, float diff);
    
    void Check_Hover(double mouseX, double mouseY);
    void Check_Click(double mouseX, double mouseY, int state);
    
    void Draw_All(std::string group = "default");
};