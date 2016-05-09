#include "Inventory.h"

#include <glm/glm.hpp>

const int SLOTS_X = 10;
const int SLOTS_Y = 6;

const int MAX_STACK_SIZE = 64;

const float START_X = 320.0f;
const float END_X = 1120.0f;

const float START_Y = 220.0f;
const float END_Y = 700.0f;

const float INV_PAD = 10.0f;
const float SLOT_PAD = 10.0f;
const float SLOT_WIDTH = 80.0f;

const float TOOLBAR_START_X = START_X + 200.0f;
const float TOOLBAR_END_X = END_X - 200.0f;

const float TOOLBAR_START_Y = 40.0f;
const float TOOLBAR_END_Y = TOOLBAR_START_Y + SLOT_WIDTH / 2.0f;

const glm::vec3 BACKGROUND_COLOR = glm::vec3(0.0f);

const glm::vec3 BORDER_COLOR = glm::vec3(0.5f);
const glm::vec3 TOOLBAR_COLOR = glm::vec3(1.0f);

const float BACKGROUND_OPACITY = 0.7f;

const std::string TEXT_GROUP = "inv";
const std::string TOOLBAR_TEXT = "toolbar";

Shader* UIShader;
Shader* UIBorderShader;
Shader* UITextureShader;

void Upload_Data(const unsigned int vbo, const Data &data) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Extend(Data &storage, const Data input) {
    for (auto const &object : input) {
        storage.push_back(object);
    }
}

Data Get_Rect(float x1, float x2, float y1, float y2) {
    return Data {x1, y1, x2, y1, x2, y2, x1, y1, x2, y2, x1, y2};
}

Data Get_Vertices(int type, float baseX, float baseY, float multiplierX, float multiplierY) {
    if (multiplierY == -1) {
        multiplierY = multiplierX;
    }
    
    static float textureStep = (1.0f / 16.0f);
    static float vertices[6][2] = { {0, 0}, {1, 0}, {1, 1}, {0, 0}, {1, 1}, {0, 1} };
    static float texCoords[6][2] = { {0, 0}, {1, 0}, {1, 1}, {0, 0}, {1, 1}, {0, 1} };
    
    glm::vec2 texPosition = textureCoords[type];
    
    Data result;
    
    for (int i = 0; i < 6; i++) {
        result.push_back(baseX + vertices[i][0] * multiplierX);
        result.push_back(baseY + vertices[i][1] * multiplierY);
        
        result.push_back((texPosition.x + texCoords[i][0] - 1.0f) * textureStep);
        result.push_back((texPosition.y + texCoords[i][1] - 1.0f) * textureStep);
    }
    
    return result;
}

void Inventory::Init() {
    for (int i = 0; i < SLOTS_X * SLOTS_Y; i++) {
        Inv.push_back(Stack(0, 0));
    }
    
    Init_UI();
}

void Inventory::Init_UI() {
    Data gridData;
    Data toolbarGridData;
    
    // Fix for missing pixel in upper left corner
    Extend(gridData, Data {START_X - 0.5f, END_Y + 0.5f, START_X + 0.5f, float(END_Y)});
    
    // Grey vertical lines
    for (float x = START_X; x <= END_X; x += SLOT_WIDTH) {
        Extend(gridData, Data {x, START_Y + SLOT_WIDTH + 1.5, x, END_Y});
    }
    
    // Grey horizontal lines
    for (float y = START_Y + SLOT_WIDTH * 2; y <= END_Y; y += SLOT_WIDTH) {
        Extend(gridData, Data {START_X, y, END_X, y});
    }
    
    // White horizontal lines
    Extend(gridData, Data {START_X - 0.5, START_Y + SLOT_WIDTH, END_X, START_Y + SLOT_WIDTH, START_X, START_Y, END_X, START_Y});
    
    // White vertical lines
    for (float x = START_X; x <= END_X; x += SLOT_WIDTH) {
        Extend(gridData, Data {x, START_Y, x, START_Y + SLOT_WIDTH});
    }
    
    
    Extend(toolbarGridData, Data {
        TOOLBAR_START_X - 0.5, TOOLBAR_END_Y, TOOLBAR_END_X, TOOLBAR_END_Y, TOOLBAR_START_X, TOOLBAR_START_Y, TOOLBAR_END_X, TOOLBAR_START_Y
    });
    
    for (float x = TOOLBAR_START_X; x <= TOOLBAR_END_X; x += SLOT_WIDTH / 2) {
        Extend(toolbarGridData, Data {x, TOOLBAR_END_Y, x, TOOLBAR_START_Y});
    }
    
    Data data[4] = {
        Get_Rect(START_X - INV_PAD, END_X + INV_PAD, START_Y - INV_PAD, END_Y + INV_PAD), // Background
        gridData, toolbarGridData,
        Get_Rect(TOOLBAR_START_X, TOOLBAR_END_X, TOOLBAR_START_Y, TOOLBAR_END_Y) // Toolbar background
    };
    
    int index = 0;
    
    for (auto const &name : BufferNames) {
        glGenVertexArrays(1, &Buffers[name].first);
        glGenBuffers(1, &Buffers[name].second);
        
        glBindVertexArray(Buffers[name].first);
        glBindBuffer(GL_ARRAY_BUFFER, Buffers[name].second);
        
        if (index < 4) {
            glBufferData(GL_ARRAY_BUFFER, data[index].size() * sizeof(float), data[index].data(), GL_STATIC_DRAW);
        }
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, false, (2 + 2 * (index >= 6)) * sizeof(float), (void*)0);
        
        if (index >= 6) {
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, false, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        }
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        
        index++;
    }
    
    Switch_Slot();
}

void Inventory::Clear() {
    for (int i = 0; i < int(Inv.size()); i++) {
        Inv[i] = Stack(0, 0);
    }
    
    Mesh();
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

void Inventory::Decrease_Size(unsigned int slot) {
    Inv[slot].second--;
    
    if (Inv[slot].second == 0) {
        Inv[slot].first = 0;
    }
    
    Mesh();
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
            int amount = int(ceil(Inv[slot].second / 2.0));
            
            Inv[slot].second -= amount;
            HoldingStack.second = amount;
            
            if (Inv[slot].second == 0) {
                Inv[slot].first = 0;
            }
        }
    }
}

void Inventory::Switch_Slot() {
    float startX = TOOLBAR_START_X + ActiveToolbarSlot * SLOT_WIDTH / 2;
    Upload_Data(Buffers["ToolbarSelect"].second, Get_Rect(startX, startX + SLOT_WIDTH / 2, TOOLBAR_START_Y, TOOLBAR_END_Y));
}

void Inventory::Swap_Stacks(Stack &a, Stack &b) {
    Stack toBePlaced = a;
    a = b;
    b = toBePlaced;
}

void Inventory::Click_Handler(double x, double y, int button) {
    if (HoveringSlot >= 0) {
        Click_Slot(HoveringSlot, button);
        Mesh();
    }
}

void Inventory::Mouse_Handler(double x, double y) {
    if (x == -1 || y == -1) {
        x = MousePos.x;
        y = MousePos.y;
    }
    
    MousePos = glm::vec2(x, y);
    
    float mouseX = float(x);
    float mouseY = float(900.0f - y);
    
    HoveringSlot = -1;
    
    if (x >= START_X && x <= END_X) {
        if (y >= START_Y && y <= END_Y) {
            float startX = float(floor((x - START_X) / SLOT_WIDTH) * SLOT_WIDTH + START_X);
            float startY = float(floor((900 - y - START_Y) / SLOT_WIDTH) * SLOT_WIDTH + START_Y);
            
            HoveringSlot = int((startY - START_Y) / SLOT_WIDTH) * SLOTS_X + int((startX - START_X) / SLOT_WIDTH);
            
            Upload_Data(Buffers["Hover"].second, Get_Rect(startX, startX + SLOT_WIDTH, startY, startY + SLOT_WIDTH));
        }
    }
    
    if (HoldingStack.first) {
        Text::Set_Group(TEXT_GROUP);
        Text::Add("holdingStack", std::to_string(HoldingStack.second), mouseY + 5);
        Text::Set_X("holdingStack", mouseX + 5);
        Text::Set_Opacity("holdingStack", 1.0f);
        Text::Unset_Group();
        
        Data data = Get_Vertices(HoldingStack.first, mouseX, mouseY, SLOT_WIDTH / 2);
        Upload_Data(Buffers["Mouse"].second, data);
        MouseVertices = int(data.size()) / 4;
    }
    else {
        MouseVertices = 0;
    }
}

void Inventory::Mesh() {
    Text::Delete_Group(TEXT_GROUP);
    Text::Delete_Group(TOOLBAR_TEXT);
    
    Data data;
    Data toolbarData;
    
    int index = 0;
    
    for (auto const &stack : Inv) {
        if (stack.first) {
            float startX = START_X + (index % SLOTS_X) * SLOT_WIDTH + SLOT_PAD;
			float startY = START_Y + (index / SLOTS_X) * SLOT_WIDTH + SLOT_PAD;
            
			float toolbarStartX = TOOLBAR_START_X + (index % SLOTS_X) * SLOT_WIDTH / 2.0f + SLOT_PAD / 2.0f;
            
            if (Is_Open) {
                Extend(data, Get_Vertices(stack.first, startX, startY, SLOT_WIDTH - SLOT_PAD * 2.0f));
            }
            else {
                Extend(toolbarData, Get_Vertices(stack.first, toolbarStartX, TOOLBAR_START_Y + SLOT_PAD / 2, SLOT_WIDTH / 2 - SLOT_PAD));
            }
            
            std::string textName = std::to_string(index);
            
            if (Is_Open) {
                Text::Set_Group(TEXT_GROUP);
                Text::Add(textName, std::to_string(stack.second), startY + 5);
                Text::Set_X(textName, startX + 5);
            }
            
            else {
                Text::Set_Group(TOOLBAR_TEXT);
                Text::Add(textName, std::to_string(stack.second), TOOLBAR_START_Y + 5);
                Text::Set_X(textName, toolbarStartX + 2);
            }
            
            Text::Set_Opacity(textName, 1.0f);
            Text::Unset_Group();
        }
        
        if (!Is_Open && index == 9) {
            break;
        }
        
        index++;
    }
    
    if (Is_Open) {
        Upload_Data(Buffers["Slots"].second, data);
        SlotVertices = int(data.size()) / 4;
    }
    
    else {
        Upload_Data(Buffers["Toolbar"].second, toolbarData);
        ToolbarSlotVertices = int(toolbarData.size()) / 4;
    }
    
    Mouse_Handler(MousePos.x, MousePos.y);
}

void Inventory::Draw() {
    if (Is_Open) {
        UIShader->Bind();
        
        glUniform3f(colorLocation, BACKGROUND_COLOR.r, BACKGROUND_COLOR.g, BACKGROUND_COLOR.b);
        glUniform1f(alphaLocation, BACKGROUND_OPACITY);
        
        glBindVertexArray(Buffers["Background"].first);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        
        glClear(GL_DEPTH_BUFFER_BIT);
        
        UIBorderShader->Bind();
        
        glBindVertexArray(Buffers["Grid"].first);
        
        glUniform3f(borderColorLocation, BORDER_COLOR.r, BORDER_COLOR.g, BORDER_COLOR.b);
        glDrawArrays(GL_LINES, 0, 34);
        
        glUniform3f(borderColorLocation, TOOLBAR_COLOR.r, TOOLBAR_COLOR.g, TOOLBAR_COLOR.b);
        glDrawArrays(GL_LINES, 34, 26);
        
        glBindVertexArray(0);
        
        UITextureShader->Bind();
        
        glBindVertexArray(Buffers["Slots"].first);
        glDrawArrays(GL_TRIANGLES, 0, SlotVertices);
        glBindVertexArray(0);
        
        if (HoldingStack.first) {
            glClear(GL_DEPTH_BUFFER_BIT);
            
            glBindVertexArray(Buffers["Mouse"].first);
            glDrawArrays(GL_TRIANGLES, 0, MouseVertices);
            glBindVertexArray(0);
        }
        
        UITextureShader->Unbind();
        
        glClear(GL_DEPTH_BUFFER_BIT);
        Text::Draw_Group(TEXT_GROUP);
        
        if (HoveringSlot >= 0) {
            glClear(GL_DEPTH_BUFFER_BIT);
            
            UIShader->Bind();
            
            glUniform3f(colorLocation, 1, 1, 1);
            glUniform1f(alphaLocation, 0.3f);
            
            glBindVertexArray(Buffers["Hover"].first);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
            
            UIShader->Unbind();
        }
    }
    else {
        UIShader->Bind();
        
        glUniform3f(colorLocation, BACKGROUND_COLOR.r, BACKGROUND_COLOR.g, BACKGROUND_COLOR.b);
        glUniform1f(alphaLocation, BACKGROUND_OPACITY);
        
        glBindVertexArray(Buffers["ToolbarBackground"].first);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        
        glUniform3f(colorLocation, 1, 1, 1);
        
        glClear(GL_DEPTH_BUFFER_BIT);
        
        glBindVertexArray(Buffers["ToolbarSelect"].first);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        
        glClear(GL_DEPTH_BUFFER_BIT);
        
        UIBorderShader->Bind();
        
        glUniform3f(borderColorLocation, TOOLBAR_COLOR.r, TOOLBAR_COLOR.g, TOOLBAR_COLOR.b);
        
        glBindVertexArray(Buffers["ToolbarGrid"].first);
        glDrawArrays(GL_LINES, 0, 26);
        glBindVertexArray(0);
        
        UITextureShader->Bind();
        
        glBindVertexArray(Buffers["Toolbar"].first);
        glDrawArrays(GL_TRIANGLES, 0, ToolbarSlotVertices);
        glBindVertexArray(0);
        
        UITextureShader->Unbind();
        
        glClear(GL_DEPTH_BUFFER_BIT);
        Text::Draw_Group(TOOLBAR_TEXT);
    }
}