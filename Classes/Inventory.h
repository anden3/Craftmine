#pragma once

#include <map>
#include <string>
#include <vector>

#define GLM_SWIZZLE
#include <glm/glm.hpp>

struct Stack {
    Stack() {
        Type = 0;
        Size = 0;
    }

    Stack(std::string type, int size = 1) : Size(size) {
        size_t delimPos = type.find(':');
        Type = std::stoi(type.substr(0, delimPos));

        if (delimPos != std::string::npos) {
            Data = std::stoi(type.substr(delimPos + 1));
        }
    }

    Stack(int type, int data, int size) : Type(type), Size(size), Data(data) {}
    Stack(int type, int size = 1) : Type(type), Size(size) {}

    void Clear() {
        Type = 0;
        Size = 0;
        Data = 0;
    }

    int Type;
    int Size;
    int Data;
};

extern bool keys[1024];

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
    void Add_Stack(int type, int typeData, int size);
    void Decrease_Size(int slot = -1);

    Stack Get_Info(int slot = -1);

    void Switch_Slot();

    void Click_Handler(int button, int action);
    void Mouse_Handler(double x = -1, double y = -1);

    void Mesh();
    void Draw();

private:
    std::vector<Stack> Inv;
    std::vector<Stack> Craft;
    std::vector<Stack> Toolbar;

    Stack CraftingOutput = Stack();
    Stack HoldingStack = Stack();

    glm::dvec2 MousePos = glm::dvec2(0);

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
