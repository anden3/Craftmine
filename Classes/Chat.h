#pragma once

#include <map>
#include <string>
#include <vector>

struct Message {
    Message() {}
    Message(unsigned int Id, float y, std::string text, double timeLeft) : ID(Id), Y(y), Text(text), TimeLeft(timeLeft) {}

    unsigned int ID;
    float Y;

    std::string Text;
    double TimeLeft;

    float RealOpacity = 1.0f;

    bool Hidden = false;
    bool OutOfView = false;
};

namespace Chat {
    extern bool Focused;
    extern bool FocusToggled;

    void Init();

    void Input(unsigned int key);
    void Key_Handler(int key);
    void Mouse_Handler(double x, double y);
    void Scroll(int direction);

    void Write(std::string text);
    void Update();
};
