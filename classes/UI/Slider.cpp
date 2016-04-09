#include "Slider.h"

#include <vector>
#include <map>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "Text.h"
#include "Shader.h"

const int padding = 20;
const float sliderWidth = 10.0f;

const glm::vec3 backgroundColor = glm::vec3(0.2f);

const glm::vec3 handleColor = glm::vec3(0.5f);
const glm::vec3 handleHoverColor = glm::vec3(0.7f);
const glm::vec3 handleClickColor = glm::vec3(0.3f, 0.3f, 0.8f);

std::string activeSlider = "";

bool Dragging = false;

struct SliderStruct {
    std::string Name;
    std::string Group;
    
    float X;
    float Y;
    float Width;
    float Height;
    
    float Min;
    float Max;
    float Value;
    
    float Position;
    float HandleX;
    
    float HandleOpacity = 1.0f;
    float BackgroundOpacity = 1.0f;
    float TextOpacity = 1.0f;
    
    glm::vec3 HandleColor = handleColor;
    glm::vec3 BackgroundColor = backgroundColor;
    glm::vec3 TextColor = glm::vec3(1.0f);
    
    unsigned int BackgroundVAO, BackgroundVBO;
    unsigned int HandleVAO, HandleVBO;
    unsigned int BorderVAO, BorderVBO;
    
    bool IsHovering = false;
    bool Active = true;
    
    Func* Function;
};

std::map<std::string, SliderStruct> Sliders;

void Slider::Add(std::string name, std::string text, Func &function, float x, float y, float w, float min, float max, float start, std::string group) {
    float h = padding * 2.0f;
    
    SliderStruct slider;
    
    slider.Name = name;
    slider.Group = group;
    
    slider.X = x;
    slider.Y = y;
    slider.Width = w;
    slider.Height = h;
    
    slider.Min = min;
    slider.Max = max;
    slider.Position = start;
    
    slider.Function = function;
    
    Text::Add(name, text);
    
    Text::Set_X(name, x + (w - Text::Get_Width(name)) / 2);
    Text::Set_Y(name, y + padding - (FONT_SIZE / 6));
    Text::Set_Color(name, slider.TextColor);
    Text::Set_Opacity(name, slider.TextOpacity);
    
    float percentage = (start - min) / (max - min);
    float sw = sliderWidth / 2;
    float sx = x + w * percentage;
    slider.HandleX = sx;
    
    std::vector<float> background = {x, y + h,  x, y,  x + w, y,  x, y + h,  x + w, y,  x + w, y + h};
    std::vector<float> handle = {sx, y + h,  sx, y,  sx + sw, y,  sx, y + h,  sx + sw, y,  sx + sw, y + h};
    std::vector<float> border = {x, y,  x + w, y,  x + w, y + h,  x - 0.5f, y + h};
    
    glGenBuffers(1, &slider.BackgroundVBO);
    glGenBuffers(1, &slider.HandleVBO);
    glGenBuffers(1, &slider.BorderVBO);
    
    glGenVertexArrays(1, &slider.BackgroundVAO);
    glGenVertexArrays(1, &slider.HandleVAO);
    glGenVertexArrays(1, &slider.BorderVAO);
    
    glBindVertexArray(slider.BackgroundVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, slider.BackgroundVBO);
    glBufferData(GL_ARRAY_BUFFER, background.size() * sizeof(float), background.data(), GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, false, 2 * sizeof(float), (void*)0);
    
    glBindVertexArray(slider.HandleVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, slider.HandleVBO);
    glBufferData(GL_ARRAY_BUFFER, handle.size() * sizeof(float), handle.data(), GL_DYNAMIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, false, 2 * sizeof(float), (void*)0);
    
    glBindVertexArray(slider.BorderVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, slider.BorderVBO);
    glBufferData(GL_ARRAY_BUFFER, border.size() * sizeof(float), border.data(), GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, false, 2 * sizeof(float), (void*)0);
    
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    Sliders[name] = slider;
}

void Slider::Delete(std::string name) {
    SliderStruct slider = Sliders[name];
    
    glDeleteBuffers(1, &slider.BackgroundVBO);
    glDeleteBuffers(1, &slider.HandleVBO);
    glDeleteBuffers(1, &slider.BorderVBO);
    
    glDeleteVertexArrays(1, &slider.BackgroundVAO);
    glDeleteVertexArrays(1, &slider.HandleVAO);
    glDeleteVertexArrays(1, &slider.BorderVAO);
}

void Slider::Draw(std::string name) {
    SliderStruct slider = Sliders[name];
    
    UIShader->Bind();
    
    glUniform1f(alphaLocation, slider.BackgroundOpacity);
    glUniform3f(colorLocation, slider.BackgroundColor.r, slider.BackgroundColor.g, slider.BackgroundColor.b);
    
    glBindVertexArray(slider.BackgroundVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    
    glClear(GL_DEPTH_BUFFER_BIT);
    
    glUniform1f(alphaLocation, slider.HandleOpacity);
    glUniform3f(colorLocation, slider.HandleColor.r, slider.HandleColor.g, slider.HandleColor.b);
    
    glBindVertexArray(slider.HandleVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    
    UIShader->Unbind();
    
    glClear(GL_DEPTH_BUFFER_BIT);
    
    UIBorderShader->Bind();
    
    glBindVertexArray(slider.BorderVAO);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
    glBindVertexArray(0);
    
    Text::Draw_String(slider.Name);
}

void Slider::Set_Text(std::string name, std::string text) {
    Text::Set_Text(Sliders[name].Name, text);
}

float Slider::Get_Value(std::string name) {
    return Sliders[name].Value;
}

void Slider::Move(std::string name, float position) {
    SliderStruct slider = Sliders[name];
    
    if (position <= slider.X + (sliderWidth / 2)) {
        position = slider.X + (sliderWidth / 2);
    }
    else if (position >= slider.X + slider.Width - 1) {
        position = slider.X + slider.Width - 1;
    }
    
    float y = slider.Y;
    float w = sliderWidth / 2;
    float h = padding * 2.0f;
    
    float percentage = (position - slider.X) / slider.Width;
    float x = slider.X + slider.Width * percentage - (sliderWidth / 2);
    
    std::vector<float> data = {x, y + h,  x, y,  x + w, y,  x, y + h,  x + w, y,  x + w, y + h};
    
    glBindBuffer(GL_ARRAY_BUFFER, slider.HandleVBO);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    Sliders[name].HandleX = x;
    Sliders[name].Value = (slider.Max - slider.Min) * percentage;
}

void Slider::Check_Hover(double mouseX, double mouseY) {
    if (Dragging) {
        Move(activeSlider, mouseX);
        Sliders[activeSlider].Function();
        return;
    }
    
    bool activeSliders = false;
    
    for (auto slider : Sliders) {
        if (slider.second.Active) {
            if (mouseX > slider.second.HandleX && mouseX < slider.second.HandleX + sliderWidth) {
                if (mouseY > slider.second.Y && mouseY < slider.second.Y + slider.second.Height) {
                    activeSliders = true;
                    
                    if (!slider.second.IsHovering) {
                        Sliders[slider.first].IsHovering = true;
                        Sliders[slider.first].HandleColor = handleHoverColor;
                        
                        activeSlider = slider.first;
                    }
                    continue;
                }
            }
        }
        
        Sliders[slider.first].IsHovering = false;
        Sliders[slider.first].HandleColor = handleColor;
    }
    
    if (!activeSliders) {
        activeSlider = "";
    }
}

void Slider::Check_Click(double mouseX, double mouseY, int state) {
    if (activeSlider != "") {
        if (state == GLFW_PRESS) {
            Dragging = true;
            Sliders[activeSlider].HandleColor = handleClickColor;
        }
        else if (state == GLFW_RELEASE) {
            Dragging = false;
            Sliders[activeSlider].HandleColor = handleHoverColor;
            Sliders[activeSlider].Function();
        }
    }
}

void Slider::Draw_All(std::string group) {
    for (auto const slider : Sliders) {
        Sliders[slider.first].Active = (slider.second.Group == group);
        if (Sliders[slider.first].Active) Draw(slider.first);
    }
}