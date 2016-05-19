#pragma once

#include "Buffer.h"
#include "Interface.h"

#include <tuple>

typedef std::vector<float> Data;

extern Interface interface;

struct Stack {
    Stack(int type = 0, int size = 1) : Type(type), Size(size) {
        if (Type == 0) {
            Size = 0;
        }
    }
    
    unsigned int Type;
    unsigned int Size;
};

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

template <typename T>
inline float X_Frac(const T a, const T b) {
    return SCREEN_WIDTH * float(a) / float(b);
}

template <typename T>
inline float Y_Frac(const T a, const T b) {
    return SCREEN_HEIGHT * float(a) / float(b);
}

class Inventory {
public:
    bool Is_Open = false;
    int ActiveToolbarSlot = 0;
    
    void Init();
    void Clear();
    
    inline void Add_Stack(Stack stack) { Add_Stack(stack.Type, stack.Size); }
    void Add_Stack(unsigned int type, unsigned int size = 1);
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
    
    glm::vec2 MousePos = glm::vec2(0, 0);
    
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