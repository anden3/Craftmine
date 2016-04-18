#pragma once

#include <string>

#include <glm/glm.hpp>

extern int SCREEN_WIDTH;
extern int SCREEN_HEIGHT;

extern const int FONT_SIZE;

struct String {
    std::string Text;
    
    float X = -1;
    float Y = -1;
    float Scale = -1;
    float Opacity = -1;
    
    float Width = 0;
    float Height;
    
    glm::vec3 Color = glm::vec3(-1);
};

namespace Text {
    void Init(std::string font, int font_size);
    
    void Add(std::string name, std::string text, float y = -1);
    void Remove(std::string name);
    void Delete_Group(std::string group);
    
    void Set_Group(std::string group);
    void Unset_Group();
    
    float Get_Width(std::string string);
    float Get_String_Width(std::string string);
    std::string Get_String_To_Width(std::string string, float width);
    
    float Get_Opacity(std::string name);
    
    void Set_Text(std::string name, std::string text);
    void Set_X(std::string name, float x);
    void Set_Y(std::string name, float y);
    void Set_Scale(std::string name, float scale);
    void Set_Opacity(std::string name, float opacity);
    void Set_Color(std::string name, glm::vec3 color);
    
    void Draw(String string);
    
    void Draw_String(std::string name);
    void Draw_Group(std::string group);
}