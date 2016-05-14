#include "Button.h"

#include "Text.h"
#include "Buffer.h"

const int padding = 20;

const glm::vec3 backgroundColor = glm::vec3(0.5f);
const glm::vec3 backgroundHoverColor = glm::vec3(0.7f);
const glm::vec3 backgroundClickColor = glm::vec3(0.3f, 0.3f, 0.8f);

std::string activeButton = "";

struct ButtonStruct {
    std::string Name;
    std::string Group;
    
    float X;
    float Y;
    float Width;
    float Height;
    
    float BackgroundOpacity = 1.0f;
    float TextOpacity = 1.0f;
    
    glm::vec3 BackgroundColor = backgroundColor;
    glm::vec3 TextColor = glm::vec3(1.0f);
    
    Buffer BackgroundBuffer;
    Buffer BorderBuffer;
    
    bool IsHovering = false;
    bool Active = true;
    
    Func* Function;
};

std::map<std::string, ButtonStruct> Buttons;

void Button::Add(std::string name, std::string text, Func &function, float x, float y, float w, std::string group) {
    float h = padding * 2;
    
    ButtonStruct button;
    
    button.Name = name;
    button.Group = group;
    
    button.X = x;
    button.Y = y;
    button.Width = w;
    button.Height = h;
    
    button.Function = function;
    
    Text::Set_Group(group);
    
    Text::Add(name, text);
    
    Text::Set_X(name, x + float((w - Text::Get_String_Width(text)) / 2));
    Text::Set_Y(name, y + padding - float(FONT_SIZE / 6));
    Text::Set_Color(name, button.TextColor);
    Text::Set_Opacity(name, button.TextOpacity);
    
    Text::Unset_Group();
    
    Data data = {x, y + h,  x, y,  x + w, y,  x, y + h,  x + w, y,  x + w, y + h};
    Data border = {x, y,  x + w, y,  x + w, y + h,  x, y + h};
    
    button.BackgroundBuffer.Init(UIShader);
    button.BorderBuffer.Init(UIBorderShader);
    
    button.BackgroundBuffer.Create(std::vector<int> {2}, data);
    button.BorderBuffer.Create(std::vector<int> {2}, border);
    
    button.BorderBuffer.VertexType = GL_LINE_LOOP;
    
    Buttons[name] = button;
}

void Button::Delete(std::string name) {
    ButtonStruct button = Buttons[name];
    
    Text::Set_Group(button.Group);
    Text::Remove(button.Name);
    Text::Unset_Group();
}

void Button::Draw(std::string name) {
    ButtonStruct button = Buttons[name];
    
    UIShader->Upload(alphaLocation, button.BackgroundOpacity);
    UIShader->Upload(colorLocation, button.BackgroundColor);
    
    button.BackgroundBuffer.Draw();
    
    glClear(GL_DEPTH_BUFFER_BIT);
    
    UIBorderShader->Upload(borderColorLocation, glm::vec3(0));
    button.BorderBuffer.Draw();
    
    Text::Set_Group(button.Group);
    Text::Draw_String(button.Name);
    Text::Unset_Group();
}

void Button::Set_Text(std::string name, std::string text) {
    Text::Set_Group(Buttons[name].Group);
    Text::Set_Text(Buttons[name].Name, text);
    Text::Unset_Group();
}

void Button::Check_Hover(double mouseX, double mouseY) {
    bool activeButtons = false;
    
    for (auto& button : Buttons) {
        if (button.second.Active) {
            if (mouseX > button.second.X && mouseX < button.second.X + button.second.Width) {
                if (mouseY > button.second.Y && mouseY < button.second.Y + button.second.Height) {
                    activeButtons = true;
                    
                    if (!button.second.IsHovering) {
                        button.second.IsHovering = true;
                        button.second.BackgroundColor = backgroundHoverColor;
                        
                        activeButton = button.first;
                    }
                    continue;
                }
            }
        }
        
        button.second.IsHovering = false;
        button.second.BackgroundColor = backgroundColor;
    }
    
    if (!activeButtons) {
        activeButton = "";
    }
}

void Button::Check_Click(double mouseX, double mouseY, int state) {
    if (activeButton != "") {
        if (state == GLFW_PRESS) {
            Buttons[activeButton].BackgroundColor = backgroundClickColor;
        }
        else if (state == GLFW_RELEASE) {
            Buttons[activeButton].BackgroundColor = backgroundHoverColor;
            Buttons[activeButton].Function();
        }
    }
}

void Button::Draw_All(std::string group) {
    for (auto& button : Buttons) {
        button.second.Active = (button.second.Group == group);
        if (button.second.Active) {
            Draw(button.first);
        }
    }
}