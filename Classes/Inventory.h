#pragma once

#include <map>
#include <string>
#include <vector>

#define GLM_SWIZZLE
#include <glm/glm.hpp>

#include <json.hpp>

typedef nlohmann::basic_json<
    std::map, std::vector, std::basic_string<
        char, std::char_traits<char>, std::allocator<char>
    >, bool, long long, double, std::allocator
> JSONValue;

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
    int Data = 0;
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

namespace Inventory {
    extern bool Is_Open;
    extern int ActiveToolbarSlot;

    extern std::vector<Stack> Inv;
    extern std::vector<Stack> Craft;

    extern Stack CraftingOutput;
    extern Stack HoldingStack;

    void Init();
    void Clear();

    void Add_Stack(int type, int typeData, int size);
    inline void Add_Stack(Stack stack) { Add_Stack(stack.Type, stack.Data, stack.Size); }
    void Decrease_Size(int slot = -1);

    Stack Get_Info(int slot = -1);

    void Switch_Slot();

    void Click_Handler(int button, int action);
    void Mouse_Handler(double x = -1, double y = -1);

    void Load(const JSONValue &data, std::vector<Stack> &storage);
    void Mesh();
    void Draw();
};
