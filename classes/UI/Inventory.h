#pragma once

#include <vector>
#include <map>

#include <glm/glm.hpp>

#include "Shader.h"
#include "Text.h"

typedef std::pair<unsigned int, unsigned int> Stack;
typedef std::pair<unsigned int, unsigned int> Buffer;
typedef std::vector<float> Data;

extern std::map<unsigned int, glm::vec2> textureCoords;

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

extern void Upload_Data(const unsigned int vbo, const Data &data);
extern void Extend(Data &storage, const Data input);

Data Get_Rect(float startX, float endX, float startY, float endY);
Data Get_Vertices(int type, float baseX, float baseY, float multiplierX, float multiplierY = -1);

class Inventory {
public:
    bool Is_Open = false;
    int ActiveToolbarSlot = 0;
    
    void Init();
    void Clear();
    
    void Add_Stack(unsigned int type, unsigned int size = 1);
    void Decrease_Size(int slot = -1);
    
    Stack Get_Info(int slot = -1);
    
    void Switch_Slot();
    
    void Click_Handler(double x, double y, int button);
    void Mouse_Handler(double x = -1, double y = -1);
    
    void Mesh();
    void Draw();

private:
    std::vector<Stack> Inv;
    std::vector<Stack> Craft;
    
    Stack CraftingOutput = Stack(0, 0);
    Stack HoldingStack = Stack(0, 0);
    
    glm::vec2 MousePos = glm::vec2(0, 0);
    
    int HoveringSlot = -1;
    
    std::vector<std::string> BufferNames = {"Background", "Grid", "ToolbarGrid", "ToolbarBackground", "ToolbarSelect", "Hover", "Slots", "Mouse", "Toolbar"};
    
    std::map<std::string, Buffer> Buffers = {
        {"Background", Buffer(0, 0)},
        {"Grid", Buffer(0, 0)},
        {"ToolbarGrid", Buffer(0, 0)},
        {"ToolbarBackground", Buffer(0, 0)},
        {"ToolbarSelect", Buffer(0, 0)},
        {"Hover", Buffer(0, 0)},
        {"Slots", Buffer(0, 0)},
        {"Mouse", Buffer(0, 0)},
        {"Toolbar", Buffer(0, 0)}
    };
    
    int SlotVertices = 0;
    int MouseVertices = 0;
    int ToolbarSlotVertices = 0;
    
    void Init_UI();
    
    void Click_Slot(unsigned int slot, int button);
    void Check_Crafting();
    void Craft_Item();
    
    void Swap_Stacks(Stack &a, Stack &b);
};