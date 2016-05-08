#pragma once

#include <string>
#include <map>
#include <vector>

#include "Shader.h"

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
    
    Chat();
    
    void Init(Shader& ui, Shader& uiBorder, unsigned int colorLoc, unsigned int alphaLoc);
    
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
    
    unsigned int BackgroundVAO, BackgroundVBO;
    unsigned int MessageVAO, MessageVBO;
    
    void Init_Chat_Background();
    
    Shader* UIShader;
    Shader* UIBorderShader;
    
    unsigned int ColorLocation;
    unsigned int AlphaLocation;
    
    void Get_Prev();
    void Get_Next();
    
    void Toggle_Cursor(int opacity = -1);
    void Update_Message();
    void Move_Up();
    void Submit();
    
    void Draw_Background();
};