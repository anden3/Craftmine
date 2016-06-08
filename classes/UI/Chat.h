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

class Chat {
  public:
    bool Focused = false;
    bool FocusToggled = false;
    
    Chat() {};
    
    void Init();
    
    void Input(unsigned int key);
    void Key_Handler(int key);
    void Mouse_Handler(double x, double y);
    void Scroll(int direction);
    
    void Write(std::string text);
    void Update();
    
  private:
    std::map<unsigned int, Message> Messages;
    std::vector<std::string> History;
    
    unsigned int MessageCount = 0;
    unsigned int HistoryIndex = 0;
    unsigned int CursorPos = 0;
    
    double LastCursorToggle = 0.0;
    bool CursorVisible = true;
    
    double MouseX;
    double MouseY;
    
    bool MouseOverChat = false;
    
    std::string NewMessage = "";
    
    void Get_Prev();
    void Get_Next();
    
    void Toggle_Cursor(float opacity = -1.0f);
    void Update_Message();
    void Move_Up(float spacing);
    void Submit();
    
    std::vector<std::string> Process_Commands(std::string message);
};