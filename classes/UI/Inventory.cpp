#include "Inventory.h"

#include <glm/glm.hpp>

const int INVENTORY_SLOTS = 50;

const int START_X = 320;
const int END_X = 1120;

const int START_Y = 220;
const int END_Y = 700;

const int INV_PAD = 10;

const glm::vec3 BACKGROUND_COLOR = glm::vec3(0.0f);
const glm::vec3 BORDER_COLOR = glm::vec3(1.0f);

const float BACKGROUND_OPACITY = 0.7f;

Shader* UIShader;
Shader* UIBorderShader;

void Inventory::Init() {
    Inv.resize(INVENTORY_SLOTS);
    Init_UI();
}

void Inventory::Init_UI() {
    glGenBuffers(1, &BackgroundVBO);
    glGenVertexArrays(1, &BackgroundVAO);
    
    glGenBuffers(1, &GridVBO);
    glGenVertexArrays(1, &GridVAO);
    
    std::vector<float> bgData {
        START_X - INV_PAD,  START_Y - INV_PAD,
        END_X   + INV_PAD,  START_Y - INV_PAD,
        END_X   + INV_PAD,  END_Y   + INV_PAD,
        START_X - INV_PAD,  START_Y - INV_PAD,
        END_X   + INV_PAD,  END_Y   + INV_PAD,
        START_X - INV_PAD,  END_Y   + INV_PAD
    };
    
    std::vector<float> gridData;
    
    for (int x = START_X; x <= END_X; x += 80) {
        gridData.push_back(x);
        gridData.push_back(START_Y);
        
        gridData.push_back(x);
        gridData.push_back(END_Y);
    }
    
    for (int y = START_Y; y <= END_Y; y += 80) {
        gridData.push_back(START_X);
        gridData.push_back(y);
        
        gridData.push_back(END_X);
        gridData.push_back(y);
    }
    
    // Fix for missing pixel in upper left corner
    gridData.push_back(START_X - 0.5f);
    gridData.push_back(END_Y + 0.5f);
    
    gridData.push_back(START_X + 0.5f);
    gridData.push_back(END_Y);
    
    glBindVertexArray(BackgroundVAO);
    glBindBuffer(GL_ARRAY_BUFFER, BackgroundVBO);
    
    glBufferData(GL_ARRAY_BUFFER, bgData.size() * sizeof(float), bgData.data(), GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, false, 2 * sizeof(float), (void*)0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    glBindVertexArray(GridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, GridVBO);
    
    glBufferData(GL_ARRAY_BUFFER, gridData.size() * sizeof(float), gridData.data(), GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, false, 2 * sizeof(float), (void*)0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Inventory::Click_Handler(double x, double y, int button) {
    
}

void Inventory::Draw() {
    UIShader->Bind();
    
    glUniform3f(colorLocation, BACKGROUND_COLOR.r, BACKGROUND_COLOR.g, BACKGROUND_COLOR.b);
    glUniform1f(alphaLocation, BACKGROUND_OPACITY);
    
    glBindVertexArray(BackgroundVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    
    UIShader->Unbind();
    
    glClear(GL_DEPTH_BUFFER_BIT);
    
    UIBorderShader->Bind();
    
    glUniform3f(borderColorLocation, BORDER_COLOR.r, BORDER_COLOR.g, BORDER_COLOR.b);
    
    glBindVertexArray(GridVAO);
    glDrawArrays(GL_LINES, 0, 38);
    glBindVertexArray(0);
    
    UIBorderShader->Unbind();
}