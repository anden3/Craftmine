#pragma once

#include <map>
#include <string>
#include <vector>

#define GLM_SWIZZLE
#include <glm/glm.hpp>

class Interface;
class Shader;

extern Interface interface;

struct Stack {
    Stack() {
        Type = 0;
        Size = 0;
    }
    
    Stack(std::string type, int size = 1) : Size(size) {
        unsigned long delimPos = type.find(':');
        Type = std::stoi(type.substr(0, delimPos));
        
        if (delimPos != std::string::npos) {
            Data = type.substr(delimPos + 1);
        }
    }
    
    Stack(int type, std::string data, int size = 1) : Type(type), Data(data), Size(size) {}
    Stack(int type, int size = 1) : Type(type), Size(size) {}
    
    int Type;
    int Size;
    
    std::string Data;
};

extern std::map<unsigned int, glm::vec2> BlockIcons;

extern bool keys[1024];

extern int colorLocation;
extern int alphaLocation;
extern int borderColorLocation;

extern int SCREEN_WIDTH, SCREEN_HEIGHT;

extern Shader* UIShader;
extern Shader* UIBorderShader;
extern Shader* UITextureShader;

template <typename T>
double Sum(const std::vector<T> &a) {
    double sum = 0;
    
    for (T const &e : a) {
        sum += e;
    }
    
    return sum;
}

class Inventory {
public:
    bool Is_Open = false;
    int ActiveToolbarSlot = 0;
    
    void Init();
    void Clear();
    
    inline void Add_Stack(Stack stack) { Add_Stack(stack.Type, stack.Data, stack.Size); }
    void Add_Stack(int type, std::string typeData, int size);
    void Decrease_Size(int slot = -1);
    
    Stack Get_Info(int slot = -1);
    
    void Switch_Slot();
    
    void Click_Handler(double x, double y, int button, int action);
    void Mouse_Handler(double x = -1, double y = -1);
    
    void Mesh();
    void Draw();

private:
    std::vector<Stack> Inv;
    std::vector<Stack> Craft;
    std::vector<Stack> Toolbar;
    
    Stack CraftingOutput = Stack();
    Stack HoldingStack = Stack();
    
    glm::vec2 MousePos = glm::vec2(0);
    
    int HoveringSlot = -1;
    
    void Click_Slot(int slot, int button);
    void Check_Crafting();
    void Craft_Item();
    
    Stack& Get_Stack(int slot);
    
    void Swap_Stacks(Stack &stack);
    bool Left_Click_Stack(Stack &a, Stack &b, bool swap = false);
    void Right_Click_Stack(Stack &stack);
    
    void Left_Drag(int slot);
};