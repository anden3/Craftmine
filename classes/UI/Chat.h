#pragma once

#include "Buffer.h"
#include "Interface.h"

extern Interface interface;

struct Message {
    unsigned int ID;
    float Y;
    
    std::string Text;
    double TimeLeft;
    
    float RealOpacity = 1.0f;
    
    bool Hidden = false;
};

extern double DeltaTime;

std::string Process_Commands(std::string message);

class Chat {
public:
    bool Focused = false;
    bool FocusToggled = false;
    
    Chat() {};
    
    void Init();
    
    void Write(std::string text);
    void Input(unsigned int key);
    void Key_Handler(int key);
    
    void Update();
    
private:
    std::map<unsigned int, Message> Messages;
    unsigned int MessageCount = 0;
    
    std::vector<std::string> History;
    unsigned int HistoryIndex = 0;
    
    double LastCursorToggle = 0.0;
    unsigned int CursorPos = 0;
    bool CursorVisible = true;
    
    std::string NewMessage = "";
    
    Buffer BackgroundBuffer;
    Buffer MessageBuffer;
    
    void Get_Prev();
    void Get_Next();
    
    void Toggle_Cursor(int opacity = -1);
    void Update_Message();
    void Move_Up();
    void Submit();
    
    void Draw_Background();
};