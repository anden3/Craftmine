#pragma once

#include <string>
#include <map>

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

class Chat {
public:
    bool Focused = false;
    bool FocusToggled = false;
    
    Chat();
    
    void Init(Shader& ui, Shader& uiBorder, unsigned int colorLoc, unsigned int alphaLoc);
    void Write(std::string text);
    void Input(unsigned int key);
    void Draw_Background();
    void Update();
    
private:
    std::map<unsigned int, Message> Messages;
    unsigned int MessageCount;
    
    std::string NewMessage = "";
    
    unsigned int BackgroundVAO, BackgroundVBO;
    unsigned int MessageVAO, MessageVBO;
    
    void Init_Chat_Background();
    
    Shader* UIShader;
    Shader* UIBorderShader;
    
    unsigned int ColorLocation;
    unsigned int AlphaLocation;
    
    void Update_Message();
    void Move_Up();
};