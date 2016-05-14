#include "Slider.h"

#include "Text.h"
#include "Buffer.h"

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
    
    Buffer BackgroundBuffer;
    Buffer HandleBuffer;
    Buffer BorderBuffer;
    
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
    
    Text::Set_X(name, x + int((w - Text::Get_String_Width(text)) / 2.0f));
    Text::Set_Y(name, y + padding - FONT_SIZE / 6);
    Text::Set_Color(name, slider.TextColor);
    Text::Set_Opacity(name, slider.TextOpacity);
    
    float percentage = (start - min) / (max - min);
    float sw = sliderWidth / 2;
    float sx = x + w * percentage;
    slider.HandleX = sx;
    
    std::vector<float> background = {x, y + h,  x, y,  x + w, y,  x, y + h,  x + w, y,  x + w, y + h};
    std::vector<float> handle = {sx, y + h,  sx, y,  sx + sw, y,  sx, y + h,  sx + sw, y,  sx + sw, y + h};
    std::vector<float> border = {x, y,  x + w, y,  x + w, y + h,  x - 0.5f, y + h};
    
    slider.BackgroundBuffer.Init(UIShader);
    slider.HandleBuffer.Init(UIShader);
    slider.BorderBuffer.Init(UIBorderShader);
    
    slider.BackgroundBuffer.Create(std::vector<int> {2}, background);
    slider.HandleBuffer.Create(std::vector<int> {2}, handle);
    slider.BorderBuffer.Create(std::vector<int> {2}, border);
    
    slider.BorderBuffer.VertexType = GL_LINE_LOOP;
    
    Sliders[name] = slider;
}

void Slider::Delete(std::string name) {
    SliderStruct slider = Sliders[name];
}

void Slider::Draw(std::string name) {
    SliderStruct slider = Sliders[name];
    
    UIShader->Upload(alphaLocation, slider.BackgroundOpacity);
    UIShader->Upload(colorLocation, slider.BackgroundColor);
    
    slider.BackgroundBuffer.Draw();
    
    glClear(GL_DEPTH_BUFFER_BIT);
    
    UIShader->Upload(alphaLocation, slider.HandleOpacity);
    UIShader->Upload(colorLocation, slider.HandleColor);
    
    slider.HandleBuffer.Draw();
    
    glClear(GL_DEPTH_BUFFER_BIT);
    
    slider.BorderBuffer.Draw();
    
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
    
    slider.HandleBuffer.Upload(data);
    
    Sliders[name].HandleX = x;
    Sliders[name].Value = (slider.Max - slider.Min) * percentage;
}

void Slider::Check_Hover(double mouseX, double mouseY) {
    if (Dragging) {
        Move(activeSlider, float(mouseX));
        Sliders[activeSlider].Function();
        return;
    }
    
    bool activeSliders = false;
    
    for (auto& slider : Sliders) {
        if (slider.second.Active) {
            if (mouseX > slider.second.HandleX && mouseX < slider.second.HandleX + sliderWidth) {
                if (mouseY > slider.second.Y && mouseY < slider.second.Y + slider.second.Height) {
                    activeSliders = true;
                    
                    if (!slider.second.IsHovering) {
                        slider.second.IsHovering = true;
                        slider.second.HandleColor = handleHoverColor;
                        
                        activeSlider = slider.first;
                    }
                    continue;
                }
            }
        }
        
        slider.second.IsHovering = false;
        slider.second.HandleColor = handleColor;
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
    for (auto& slider : Sliders) {
        slider.second.Active = (slider.second.Group == group);
        if (slider.second.Active) Draw(slider.first);
    }
}