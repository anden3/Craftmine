#pragma once

#include <vector>

#include "Shader.h"

struct InventorySlot {
    unsigned char Slot;
    unsigned char Type;
    unsigned int Size;
};

extern int colorLocation;
extern int alphaLocation;
extern int borderColorLocation;

extern Shader* UIShader;
extern Shader* UIBorderShader;

class Inventory {
public:
    void Init();
    void Click_Handler(double x, double y, int button);
    void Draw();

private:
    std::vector<InventorySlot> Inv;
    
    unsigned int BackgroundVBO;
    unsigned int BackgroundVAO;
    
    unsigned int GridVBO;
    unsigned int GridVAO;
    
    void Init_UI();
};