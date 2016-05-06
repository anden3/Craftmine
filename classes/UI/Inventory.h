#pragma once

#include <vector>
#include <map>

#include <glm/glm.hpp>

#include "Shader.h"
#include "Text.h"

typedef std::pair<unsigned char, unsigned int> Stack;

extern std::map<unsigned char, glm::vec2> textureCoords;

extern int colorLocation;
extern int alphaLocation;
extern int borderColorLocation;

extern Shader* UIShader;
extern Shader* UIBorderShader;
extern Shader* UITextureShader;

class Inventory {
public:
    bool Is_Open;
    
    void Init();
    
    void Add_Stack(unsigned char type, unsigned int size);
    
    void Click_Handler(double x, double y, int button);
    void Mouse_Handler(double x, double y);
    
    void Open();
    void Draw();

private:
    std::vector<Stack> Inv;
    Stack HoldingStack = Stack(0, 0);
    
    glm::vec2 MousePos = glm::vec2(0, 0);
    
    unsigned int BackgroundVBO, BackgroundVAO;
    unsigned int GridVBO, GridVAO;
    unsigned int SlotsVBO, SlotsVAO;
    unsigned int MouseVBO, MouseVAO;
    
    unsigned int VertexCounts[4] = {0};
    
    void Init_UI();
    
    void Click_Slot(unsigned int slot, int button);
    
    void Swap_Stacks(Stack &a, Stack &b);
    
    void Mesh();
};