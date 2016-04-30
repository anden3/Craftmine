#include "Button.h"

#include <vector>
#include <map>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Text.h"
#include "Shader.h"

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
    
    unsigned int BackgroundVAO, BackgroundVBO;
    unsigned int BorderVAO, BorderVBO;
    
    bool IsHovering = false;
    bool Active = true;
    
    Func* Function;
};

std::map<std::string, ButtonStruct> Buttons;

void Button::Add(std::string name, std::string text, Func &function, int x, int y, int w, std::string group) {
    int h = padding * 2;
    
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
    
    Text::Set_X(name, x + int((w - Text::Get_String_Width(text)) / 2.0f));
    Text::Set_Y(name, y + padding - FONT_SIZE / 6);
    Text::Set_Color(name, button.TextColor);
    Text::Set_Opacity(name, button.TextOpacity);
    
    Text::Unset_Group();
    
    std::vector<int> data = {x, y + h,  x, y,  x + w, y,  x, y + h,  x + w, y,  x + w, y + h};
    std::vector<int> border = {x, y,  x + w, y,  x + w, y + h,  x, y + h};
    
    glGenBuffers(1, &button.BackgroundVBO);
    glGenBuffers(1, &button.BorderVBO);
    
    glGenVertexArrays(1, &button.BackgroundVAO);
    glGenVertexArrays(1, &button.BorderVAO);
    
    glBindVertexArray(button.BackgroundVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, button.BackgroundVBO);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(int), data.data(), GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_INT, GL_FALSE, 2 * sizeof(int), (void*)0);
    
    glBindVertexArray(button.BorderVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, button.BorderVBO);
    glBufferData(GL_ARRAY_BUFFER, border.size() * sizeof(int), border.data(), GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_INT, GL_FALSE, 2 * sizeof(int), (void*)0);
    
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    Buttons[name] = button;
}

void Button::Delete(std::string name) {
    ButtonStruct button = Buttons[name];
    
    Text::Set_Group(button.Group);
    Text::Remove(button.Name);
    Text::Unset_Group();
    
    glDeleteBuffers(1, &button.BackgroundVBO);
    glDeleteBuffers(1, &button.BorderVBO);
    
    glDeleteVertexArrays(1, &button.BackgroundVAO);
    glDeleteVertexArrays(1, &button.BorderVAO);
}

void Button::Draw(std::string name) {
    ButtonStruct button = Buttons[name];
    
    UIShader->Bind();
    
    glUniform1f(alphaLocation, button.BackgroundOpacity);
    glUniform3f(colorLocation, button.BackgroundColor.r, button.BackgroundColor.g, button.BackgroundColor.b);
    
    glBindVertexArray(button.BackgroundVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    
    UIShader->Unbind();
    
    glClear(GL_DEPTH_BUFFER_BIT);
    
    UIBorderShader->Bind();
    
    glUniform3f(borderColorLocation, 0, 0, 0);
    
    glBindVertexArray(button.BorderVAO);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
    glBindVertexArray(0);
    
    Text::Set_Group(button.Group);
    Text::Draw_String(button.Name);
    Text::Unset_Group();
}

void Button::Set_Text(std::string name, std::string text) {
    Text::Set_Text(Buttons[name].Name, text);
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