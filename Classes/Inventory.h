#pragma once

#include <map>
#include <string>
#include <vector>

#define GLM_SWIZZLE
#include <glm/glm.hpp>

#include <json.hpp>

#include "Stack.h"

class Slot;

typedef nlohmann::basic_json<
    std::map, std::vector, std::basic_string<
        char, std::char_traits<char>, std::allocator<char>
    >, bool, long long, double, std::allocator
> JSONValue;

extern bool keys[1024];

template <typename T>
double Sum(const std::vector<T> &a) {
    double sum = 0;

    for (T const &e : a) {
        sum += e;
    }

    return sum;
}

namespace Inventory {
    extern bool Is_Open;
    extern int ActiveToolbarSlot;

    extern Slot* CraftingOutput;
    extern Stack HoldingStack;

    void Init();
    void Clear();

    void Click_Slot(Slot* slot);

    void Press_Slot(Slot* slot, int button);
    void Dragging_Slot(Slot* slot);
    void Release_Slot();

    void Add_Stack(int type, int typeData, int size);
    inline void Add_Stack(Stack* stack) { Add_Stack(stack->Type, stack->Data, stack->Size); }
    void Decrease_Size(int slot = -1);

    Stack* Get_Info(int slot = -1);

    void Switch_Slot(int slot);

    void Mouse_Handler(double x = -1, double y = -1);

    void Save(nlohmann::json &dest, std::string type);
    void Load(const JSONValue &data);
    void Draw();
};
