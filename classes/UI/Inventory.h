#pragma once

#include <vector>
#include <map>

#include <glm/glm.hpp>

#include "Shader.h"
#include "Text.h"

typedef std::pair<unsigned char, unsigned int> Stack;
typedef std::pair<unsigned int, unsigned int> Buffer;
typedef std::vector<float> Data;

extern std::map<unsigned char, glm::vec2> textureCoords;

extern int colorLocation;
extern int alphaLocation;
extern int borderColorLocation;

extern Shader* UIShader;
extern Shader* UIBorderShader;
extern Shader* UITextureShader;

void Upload_Data(const unsigned int vbo, const Data &data);
void Extend(Data &storage, const Data input);

Data Get_Rect(float startX, float endX, float startY, float endY);
Data Get_Vertices(int type, float baseX, float baseY, float multiplierX, float multiplierY = -1);

class Inventory {
public:
    bool Is_Open = false;
    int ActiveToolbarSlot = 0;
    
    void Init();
    void Clear();
    
    void Add_Stack(unsigned char type, unsigned int size);
    
    void Decrease_Size(unsigned int slot);
    
    inline Stack Get_Info(unsigned int slot) {
        return Inv[slot];
    }
    
    void Switch_Slot();
    
    void Click_Handler(double x, double y, int button);
    void Mouse_Handler(double x = -1, double y = -1);
    
    void Mesh();
    void Draw();

private:
    std::vector<Stack> Inv;
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
    
    void Swap_Stacks(Stack &a, Stack &b);
};