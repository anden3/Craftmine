#include "Inventory.h"

#include <glm/glm.hpp>

const int SLOTS_X = 10;
const int SLOTS_Y = 6;
const int INVENTORY_SLOTS = SLOTS_X * SLOTS_Y;

const int MAX_STACK_SIZE = 64;

const int START_X = 320;
const int END_X = 1120;

const int START_Y = 220;
const int END_Y = 700;

const int INV_PAD = 10;
const int SLOT_PAD = 10;
const int SLOT_WIDTH = 80;

const glm::vec3 BACKGROUND_COLOR = glm::vec3(0.0f);
const glm::vec3 BORDER_COLOR = glm::vec3(1.0f);

const float BACKGROUND_OPACITY = 0.7f;

const std::string TEXT_GROUP = "inv";

Shader* UIShader;
Shader* UIBorderShader;
Shader* UITextureShader;

void Inventory::Init() {
    for (int i = 0; i < INVENTORY_SLOTS; i++) {
        Inv.push_back(Stack(0, 0));
    }
    
    Inv[0] = Stack(1, 10);
    Inv[1] = Stack(2, 10);
    
    Init_UI();
}

void Inventory::Init_UI() {
    std::vector<float> bgData {
        START_X - INV_PAD,  START_Y - INV_PAD,
        END_X   + INV_PAD,  START_Y - INV_PAD,
        END_X   + INV_PAD,  END_Y   + INV_PAD,
        START_X - INV_PAD,  START_Y - INV_PAD,
        END_X   + INV_PAD,  END_Y   + INV_PAD,
        START_X - INV_PAD,  END_Y   + INV_PAD
    };
    
    std::vector<float> gridData;
    
    for (int x = START_X; x <= END_X; x += SLOT_WIDTH) {
        gridData.push_back(x);
        gridData.push_back(START_Y);
        
        gridData.push_back(x);
        gridData.push_back(END_Y);
    }
    
    for (int y = START_Y; y <= END_Y; y += SLOT_WIDTH) {
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
    
    glGenVertexArrays(1, &BackgroundVAO);
    glGenBuffers(1, &BackgroundVBO);
    
    glGenVertexArrays(1, &GridVAO);
    glGenBuffers(1, &GridVBO);
    
    glGenVertexArrays(1, &SlotsVAO);
    glGenBuffers(1, &SlotsVBO);
    
    glGenVertexArrays(1, &MouseVAO);
    glGenBuffers(1, &MouseVBO);
    
    unsigned int vaos[4] = {BackgroundVAO, GridVAO, SlotsVAO, MouseVAO};
    unsigned int vbos[4] = {BackgroundVBO, GridVBO, SlotsVBO, MouseVBO};
    
    std::vector<float> data[2] = {bgData, gridData};
    
    for (int i = 0; i < 4; i++) {
        glBindVertexArray(vaos[i]);
        glBindBuffer(GL_ARRAY_BUFFER, vbos[i]);
        
        if (i < 2) {
            glBufferData(GL_ARRAY_BUFFER, data[i].size() * sizeof(float), data[i].data(), GL_STATIC_DRAW);
            VertexCounts[i] = int(data[i].size()) / 2;
        }
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, false, (2 + 2 * (i >= 2)) * sizeof(float), (void*)0);
        
        if (i >= 2) {
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, false, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        }
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
}

void Inventory::Add_Stack(unsigned char type, unsigned int size) {
    int index = 0;
    
    for (auto const &stack : Inv) {
        if (stack.first == type) {
            if (stack.second < MAX_STACK_SIZE) {
                if (stack.second + size <= MAX_STACK_SIZE) {
                    Inv[index].second += size;
                    Mesh();
                    return;
                }
                else {
                    size -= MAX_STACK_SIZE - stack.second;
                    Inv[index].second = MAX_STACK_SIZE;
                }
            }
        }
        
        index++;
    }
    
    index = 0;
    
    for (auto const &stack : Inv) {
        if (stack.second == 0) {
            if (size <= MAX_STACK_SIZE) {
                Inv[index] = Stack(type, size);
                Mesh();
                return;
            }
            else {
                Inv[index] = Stack(type, MAX_STACK_SIZE);
                size -= MAX_STACK_SIZE;
            }
        }
        
        index++;
    }
}

void Inventory::Click_Slot(unsigned int slot, int button) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (HoldingStack.first && HoldingStack.first == Inv[slot].first) {
            if (Inv[slot].second < MAX_STACK_SIZE) {
                if (HoldingStack.second <= MAX_STACK_SIZE - Inv[slot].second) {
                    Inv[slot].second += HoldingStack.second;
                    HoldingStack = Stack(0, 0);
                    return;
                }
                else {
                    int remainder = HoldingStack.second - (MAX_STACK_SIZE - Inv[slot].second);
                    Inv[slot].second = MAX_STACK_SIZE;
                    HoldingStack.second = remainder;
                    return;
                }
            }
        }
        
        Swap_Stacks(HoldingStack, Inv[slot]);
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (HoldingStack.first) {
            if (Inv[slot].first == HoldingStack.first && Inv[slot].second < MAX_STACK_SIZE) {
                Inv[slot].second++;
                HoldingStack.second--;
            }
            else if (!Inv[slot].first) {
                Inv[slot] = Stack(HoldingStack.first, 1);
                HoldingStack.second--;
            }
            
            if (HoldingStack.second == 0) {
                HoldingStack.first = 0;
            }
        }
        else if (Inv[slot].first) {
            HoldingStack.first = Inv[slot].first;
            int amount = ceil(Inv[slot].second / 2.0);
            
            Inv[slot].second -= amount;
            HoldingStack.second = amount;
            
            if (Inv[slot].second == 0) {
                Inv[slot].first = 0;
            }
        }
    }
}

void Inventory::Swap_Stacks(Stack &a, Stack &b) {
    Stack toBePlaced = a;
    a = b;
    b = toBePlaced;
}

void Inventory::Click_Handler(double x, double y, int button) {
    if (x >= START_X && x <= END_X) {
        if (y >= START_Y && y <= END_Y) {
            int slot = (((SLOTS_Y - 1) - (int(y) - START_Y) / SLOT_WIDTH) * SLOTS_X + (int(x) - START_X) / SLOT_WIDTH);
            
            Click_Slot(slot, button);
            Mesh();
        }
    }
}

void Inventory::Mouse_Handler(double x, double y) {
    MousePos = glm::vec2(x, y);
    
    float mouseX(x);
    float mouseY(900 - y);
    
    if (HoldingStack.first) {
        std::vector<float> data;
        
        static float textureStep = (1.0f / 16.0f);
        static float vertices[6][2] = { {0, 0}, {1, 0}, {1, 1}, {0, 0}, {1, 1}, {0, 1} };
        static float texCoords[6][2] = { {0, 0}, {1, 0}, {1, 1}, {0, 0}, {1, 1}, {0, 1} };
        
        glm::vec2 texPosition = textureCoords[HoldingStack.first];
        
        for (int i = 0; i < 6; i++) {
            data.push_back(mouseX + vertices[i][0] * (SLOT_WIDTH / 2));
            data.push_back(mouseY + vertices[i][1] * (SLOT_WIDTH / 2));
            
            data.push_back((texPosition.x + texCoords[i][0]) * textureStep);
            data.push_back((texPosition.y + texCoords[i][1]) * textureStep);
        }
        
        Text::Set_Group(TEXT_GROUP);
        
        Text::Add("holdingStack", std::to_string(HoldingStack.second), mouseY + 5);
        Text::Set_X("holdingStack", mouseX + 5);
        Text::Set_Opacity("holdingStack", 1.0f);
        
        Text::Unset_Group();
        
        glBindBuffer(GL_ARRAY_BUFFER, MouseVBO);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        VertexCounts[3] = int(data.size()) / 4;
    }
    else {
        VertexCounts[3] = 0;
    }
}

void Inventory::Mesh() {
    Text::Delete_Group(TEXT_GROUP);
    
    static float textureStep = (1.0f / 16.0f);
    static float vertices[6][2] = { {0, 0}, {1, 0}, {1, 1}, {0, 0}, {1, 1}, {0, 1} };
    static float texCoords[6][2] = { {0, 0}, {1, 0}, {1, 1}, {0, 0}, {1, 1}, {0, 1} };
    
    std::vector<float> data;
    
    int index = 0;
    
    Text::Set_Group(TEXT_GROUP);
    
    for (auto const &stack : Inv) {
        if (stack.first) {
            int startX = START_X + (index % SLOTS_X) * SLOT_WIDTH + SLOT_PAD;
            int startY = START_Y + (index / SLOTS_X) * SLOT_WIDTH + SLOT_PAD;
            
            glm::vec2 texPosition = textureCoords[stack.first];
            
            for (int i = 0; i < 6; i++) {
                data.push_back(startX + vertices[i][0] * (SLOT_WIDTH - SLOT_PAD * 2));
                data.push_back(startY + vertices[i][1] * (SLOT_WIDTH - SLOT_PAD * 2));
                
                data.push_back((texPosition.x + texCoords[i][0]) * textureStep);
                data.push_back((texPosition.y + texCoords[i][1]) * textureStep);
            }
            
            Text::Add(std::to_string(index), std::to_string(stack.second), startY + 5);
            Text::Set_X(std::to_string(index), startX + 5);
            Text::Set_Opacity(std::to_string(index), 1.0f);
        }
        
        index++;
    }
    
    Text::Unset_Group();
    
    glBindBuffer(GL_ARRAY_BUFFER, SlotsVBO);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    VertexCounts[2] = int(data.size()) / 4;
    
    Mouse_Handler(MousePos.x, MousePos.y);
}

void Inventory::Open() {
    Mesh();
}

void Inventory::Draw() {
    UIShader->Bind();
    
    glUniform3f(colorLocation, BACKGROUND_COLOR.r, BACKGROUND_COLOR.g, BACKGROUND_COLOR.b);
    glUniform1f(alphaLocation, BACKGROUND_OPACITY);
    
    glBindVertexArray(BackgroundVAO);
    glDrawArrays(GL_TRIANGLES, 0, VertexCounts[0]);
    glBindVertexArray(0);
    
    glClear(GL_DEPTH_BUFFER_BIT);
    
    UIBorderShader->Bind();
    
    glUniform3f(borderColorLocation, BORDER_COLOR.r, BORDER_COLOR.g, BORDER_COLOR.b);
    
    glBindVertexArray(GridVAO);
    glDrawArrays(GL_LINES, 0, VertexCounts[1]);
    glBindVertexArray(0);
    
    UITextureShader->Bind();
    
    glBindVertexArray(SlotsVAO);
    glDrawArrays(GL_TRIANGLES, 0, VertexCounts[2]);
    glBindVertexArray(0);
    
    glBindVertexArray(MouseVAO);
    glDrawArrays(GL_TRIANGLES, 0, VertexCounts[3]);
    glBindVertexArray(0);
    
    UITextureShader->Unbind();
    
    glClear(GL_DEPTH_BUFFER_BIT);
    
    Text::Draw_Group(TEXT_GROUP);
}