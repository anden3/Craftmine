#pragma once

#include <string>

typedef void (Func)(void);

namespace Button {
    void Add(std::string name, std::string text, Func &function, float x, float y, float w, std::string group = "default");
    void Delete(std::string name);
    void Draw(std::string name);
    
    void Set_Text(std::string name, std::string text);
    
    void Check_Hover(double mouseX, double mouseY);
    void Check_Click(double mouseX, double mouseY, int state);
    
    void Draw_All(std::string group = "default");
    void Clean();
};