#pragma once

#include <map>
#include <string>
#include <vector>

class Player;
class Interface;
class Inventory;

extern Player player;
extern Interface interface;
extern Inventory inventory;

extern double DeltaTime;

struct Message {
    Message(unsigned int Id, float y, std::string text, double timeLeft) : ID(Id), Y(y), Text(text), TimeLeft(timeLeft) {}
    
    unsigned int ID;
    float Y;
    
    std::string Text;
    double TimeLeft;
    
    float RealOpacity = 1.0f;
    
    bool Hidden = false;
};

class Chat {
public:
    bool Focused = false;
    bool FocusToggled = false;
    
    Chat() {};
    
    void Init();
    
    void Input(unsigned int key);
    void Key_Handler(int key);
    
    void Update();
    
private:
    std::map<unsigned int, Message> Messages;
    std::vector<std::string> History;
    
    unsigned int MessageCount = 0;
    unsigned int HistoryIndex = 0;
    unsigned int CursorPos = 0;
    
    double LastCursorToggle = 0.0;
    bool CursorVisible = true;
    
    std::string NewMessage = "";
    
    void Get_Prev();
    void Get_Next();
    
    void Toggle_Cursor(int opacity = -1);
    void Update_Message();
    void Move_Up();
    void Submit();
    
    void Write(std::string text);
    
    std::vector<std::string> Process_Commands(std::string message);
};