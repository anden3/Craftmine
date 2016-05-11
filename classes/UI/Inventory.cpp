#include "Inventory.h"

#include <glm/glm.hpp>

typedef std::vector<unsigned char> Recipe;

const int SLOTS_X = 10;
const int SLOTS_Y = 6;

const int MAX_STACK_SIZE = 64;

float START_X, END_X;
float START_Y, END_Y;

float TOOLBAR_START_X, TOOLBAR_END_X;
float TOOLBAR_START_Y, TOOLBAR_END_Y;

float CRAFTING_START_X, CRAFTING_END_X;
float CRAFTING_START_Y;

float OUTPUT_START_X, OUTPUT_END_X;
float OUTPUT_START_Y, OUTPUT_END_Y;

float SLOT_WIDTH;

const float INV_PAD = 10.0f;
const float SLOT_PAD = 10.0f;
const float TEXT_PAD = 5.0f;

const int OUTPUT_SLOT = SLOTS_X * SLOTS_Y + 10;

const glm::vec3 BACKGROUND_COLOR = glm::vec3(0.0f);

const glm::vec3 BORDER_COLOR = glm::vec3(0.5f);
const glm::vec3 TOOLBAR_COLOR = glm::vec3(1.0f);

const float BACKGROUND_OPACITY = 0.7f;

const std::string TEXT_GROUP = "inv";
const std::string TOOLBAR_TEXT = "toolbar";

Shader* UIShader;
Shader* UIBorderShader;
Shader* UITextureShader;

std::map<Recipe, Stack> Recipes_1x1 = {
    {Recipe {}, Stack(5, 4)}
};

std::map<Recipe, Stack> Recipes_1x2 = {};

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
    START_X = SCREEN_WIDTH * 11.0f / 72.0f;
    END_X = SCREEN_WIDTH * 17.0f / 24.0f;
    
    START_Y = SCREEN_HEIGHT * 11.0f / 45.0f;
    END_Y = SCREEN_HEIGHT * 7.0f / 9.0f;
    
    TOOLBAR_START_X = SCREEN_WIDTH * 13.0f / 36.0f;
    TOOLBAR_END_X = SCREEN_WIDTH * 23.0f / 36.0f;
    
    TOOLBAR_START_Y = SCREEN_HEIGHT * 2.0f / 45.0f;
    TOOLBAR_END_Y = SCREEN_HEIGHT * 4.0f / 45.0f;
    
    CRAFTING_START_X = SCREEN_WIDTH * 55.0f / 72.0f;
    CRAFTING_END_X = SCREEN_WIDTH * 67.0f / 72.0f;
    
    CRAFTING_START_Y = SCREEN_HEIGHT * 23.0f / 45.0f;
    
    SLOT_WIDTH = SCREEN_WIDTH * 1.0f / 18.0f;
    
    OUTPUT_START_X = CRAFTING_START_X + SLOT_WIDTH;
    OUTPUT_END_X = CRAFTING_END_X - SLOT_WIDTH;
    
    OUTPUT_START_Y = CRAFTING_START_Y - SLOT_WIDTH * 2;
    OUTPUT_END_Y = CRAFTING_START_Y - SLOT_WIDTH;
    
    for (int i = 0; i < SLOTS_X * SLOTS_Y; i++) {
        Inv.push_back(Stack(0, 0));
    }
    
    for (int i = 0; i < 9; i++) {
        Craft.push_back(Stack(0, 0));
    }
    
    Init_UI();
}

void Inventory::Init_UI() {
    Data gridData;
    Data toolbarGridData;
    
    // Fix for missing pixel in upper left corner
    Extend(gridData, Data {START_X - 0.5f, END_Y + 0.5f, START_X + 0.5f, END_Y});
    Extend(gridData, Data {CRAFTING_START_X - 0.5f, END_Y + 0.5f, CRAFTING_START_X + 0.5f, END_Y});
    
    // Grey vertical lines
    for (float x = START_X; x <= END_X; x += SLOT_WIDTH) {
        Extend(gridData, Data {x, START_Y + SLOT_WIDTH + 1.5f, x, END_Y});
    }
    
    for (float x = CRAFTING_START_X; x <= CRAFTING_END_X; x += SLOT_WIDTH) {
        Extend(gridData, Data {x, CRAFTING_START_Y, x, END_Y});
    }
    
    // Grey horizontal lines
    for (float y = START_Y + SLOT_WIDTH * 2; y <= END_Y; y += SLOT_WIDTH) {
        Extend(gridData, Data {START_X, y, END_X, y});
    }
    
    for (float y = CRAFTING_START_Y; y <= END_Y; y += SLOT_WIDTH) {
        Extend(gridData, Data {CRAFTING_START_X, y, CRAFTING_END_X, y});
    }
    
    // White horizontal lines
    Extend(gridData, Data {START_X - 0.5f, START_Y + SLOT_WIDTH, END_X, START_Y + SLOT_WIDTH, START_X, START_Y, END_X, START_Y});
    
    // White vertical lines
    for (float x = START_X; x <= END_X; x += SLOT_WIDTH) {
        Extend(gridData, Data {x, START_Y, x, START_Y + SLOT_WIDTH});
    }
    
    // Crafting output
    Extend(gridData, Data {OUTPUT_START_X - 0.5f, OUTPUT_END_Y,   OUTPUT_END_X, OUTPUT_END_Y});
    Extend(gridData, Data {OUTPUT_START_X - 0.5f, OUTPUT_START_Y, OUTPUT_END_X, OUTPUT_START_Y});
    
    Extend(gridData, Data {OUTPUT_START_X - 0.5f, OUTPUT_END_Y, OUTPUT_START_X - 0.5f, OUTPUT_START_Y});
    Extend(gridData, Data {OUTPUT_END_X, OUTPUT_END_Y, OUTPUT_END_X, OUTPUT_START_Y});
    
    Extend(toolbarGridData, Data {
        TOOLBAR_START_X - 0.5f, TOOLBAR_END_Y, TOOLBAR_END_X, TOOLBAR_END_Y, TOOLBAR_START_X, TOOLBAR_START_Y, TOOLBAR_END_X, TOOLBAR_START_Y
    });
    
    for (float x = TOOLBAR_START_X; x <= TOOLBAR_END_X; x += SLOT_WIDTH / 2) {
        Extend(toolbarGridData, Data {x, TOOLBAR_END_Y, x, TOOLBAR_START_Y});
    }
    
    Data bgData = Get_Rect(START_X - INV_PAD, END_X + INV_PAD, START_Y - INV_PAD, END_Y + INV_PAD);
    Extend(bgData, Get_Rect(CRAFTING_START_X - INV_PAD, CRAFTING_END_X + INV_PAD, CRAFTING_START_Y - INV_PAD, END_Y + INV_PAD));
    Extend(bgData, Get_Rect(OUTPUT_START_X - INV_PAD, OUTPUT_END_X + INV_PAD, OUTPUT_START_Y - INV_PAD, OUTPUT_END_Y + INV_PAD));
    
    Data data[4] = {
        bgData, gridData, toolbarGridData,
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

void Inventory::Decrease_Size(int slot) {
    if (slot == -1) {
        slot = ActiveToolbarSlot;
    }
    
    Inv[slot].second--;
    
    if (Inv[slot].second == 0) {
        Inv[slot].first = 0;
    }
    
    Mesh();
}

Stack Inventory::Get_Info(int slot) {
    if (slot == -1) {
        slot = ActiveToolbarSlot;
    }
    
    return Inv[slot];
}

void Inventory::Click_Slot(unsigned int slot, int button) {
    if (slot == OUTPUT_SLOT) {
        if (CraftingOutput.first) {
            if (button == GLFW_MOUSE_BUTTON_LEFT) {
                if (keys[GLFW_KEY_LEFT_SHIFT]) {
                    Add_Stack(CraftingOutput.first, CraftingOutput.second);
                    CraftingOutput = Stack(0, 0);
                }
                
                else if (!HoldingStack.first) {
                    HoldingStack = CraftingOutput;
                    CraftingOutput = Stack(0, 0);
                }
                else if (CraftingOutput.first == HoldingStack.first) {
                    if (CraftingOutput.second + HoldingStack.second <= MAX_STACK_SIZE) {
                        HoldingStack.second += HoldingStack.second;
                        CraftingOutput = Stack(0, 0);
                    }
                    else {
                        int remainder = CraftingOutput.second - (MAX_STACK_SIZE - HoldingStack.second);
                        HoldingStack.second = MAX_STACK_SIZE;
                        CraftingOutput.second = remainder;
                    }
                }
            }
            else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
                if (!HoldingStack.first) {
                    HoldingStack.first = CraftingOutput.first;
                    HoldingStack.second = int(ceil(CraftingOutput.second / 2));
                    CraftingOutput.second -= HoldingStack.second;
                    
                    if (CraftingOutput.second == 0) {
                        CraftingOutput.first = 0;
                    }
                }
            }
        }
    }
    
    else if (slot >= SLOTS_X * SLOTS_Y) {
        slot -= SLOTS_X * SLOTS_Y;
        
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (HoldingStack.first && HoldingStack.first == Craft[slot].first) {
                if (Craft[slot].second < MAX_STACK_SIZE) {
                    if (HoldingStack.second <= MAX_STACK_SIZE - Craft[slot].second) {
                        Craft[slot].second += HoldingStack.second;
                        HoldingStack = Stack(0, 0);
                    }
                    else {
                        int remainder = HoldingStack.second - (MAX_STACK_SIZE - Craft[slot].second);
                        Craft[slot].second = MAX_STACK_SIZE;
                        HoldingStack.second = remainder;
                    }
                }
                else {
                    Swap_Stacks(HoldingStack, Craft[slot]);
                }
            }
            else {
                Swap_Stacks(HoldingStack, Craft[slot]);
            }
        }
        
        else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            if (HoldingStack.first) {
                if (Craft[slot].first == HoldingStack.first && Craft[slot].second < MAX_STACK_SIZE) {
                    Craft[slot].second++;
                    HoldingStack.second--;
                }
                else if (!Craft[slot].first) {
                    Craft[slot] = Stack(HoldingStack.first, 1);
                    HoldingStack.second--;
                }
                
                if (HoldingStack.second == 0) {
                    HoldingStack.first = 0;
                }
            }
            else if (Craft[slot].first) {
                HoldingStack.first = Craft[slot].first;
                HoldingStack.second = int(ceil(Craft[slot].second / 2.0));;
                Craft[slot].second -= HoldingStack.second;
                
                if (Craft[slot].second == 0) {
                    Craft[slot].first = 0;
                }
            }
        }
        
        Check_Crafting();
    }
    
    else {
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
                HoldingStack.second = int(ceil(Inv[slot].second / 2.0));
                Inv[slot].second -= HoldingStack.second;
                
                if (Inv[slot].second == 0) {
                    Inv[slot].first = 0;
                }
            }
        }
    }
}

void Inventory::Check_Crafting() {
    int dims[2];
    
    for (int i = 0; i < 2; i++) {
        std::vector<bool> v = {0, 0, 0};
        
        for (int a = 0; a < 3; a++) {
            for (int b = 0; b < 3; b++) {
                if (!v[b]) {
                    v[b] = i ? Craft[b * 3 + a].first > 0 : Craft[a * 3 + b].first > 0;
                }
            }
        }
        
        dims[i] = int(Sum(v));
        
        if (dims[i] == 2 && v == std::vector<bool> {1, 0, 1}) {
            dims[i] = 3;
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
    
    if (y >= START_Y && y <= END_Y) {
        if (x >= START_X && x <= END_X) {
            float startX = float(floor((x - START_X) / SLOT_WIDTH) * SLOT_WIDTH + START_X);
            float startY = float(floor((900 - y - START_Y) / SLOT_WIDTH) * SLOT_WIDTH + START_Y);
            
            HoveringSlot = int((startY - START_Y) / SLOT_WIDTH) * SLOTS_X + int((startX - START_X) / SLOT_WIDTH);
            
            Upload_Data(Buffers["Hover"].second, Get_Rect(startX, startX + SLOT_WIDTH, startY, startY + SLOT_WIDTH));
        }
        
        else if (x >= CRAFTING_START_X && x <= CRAFTING_END_X) {
            if (mouseY >= CRAFTING_START_Y) {
                float startX = float(floor((x - CRAFTING_START_X) / SLOT_WIDTH) * SLOT_WIDTH + CRAFTING_START_X);
                float startY = float(floor((900 - y - CRAFTING_START_Y) / SLOT_WIDTH) * SLOT_WIDTH + CRAFTING_START_Y);
                
                HoveringSlot = SLOTS_X * SLOTS_Y + int((startY - CRAFTING_START_Y) / SLOT_WIDTH) * 3 + int((startX - CRAFTING_START_X) / SLOT_WIDTH);
                
                Upload_Data(Buffers["Hover"].second, Get_Rect(startX, startX + SLOT_WIDTH, startY, startY + SLOT_WIDTH));
            }
            
            else if (mouseY >= OUTPUT_START_Y && mouseY <= OUTPUT_END_Y) {
                if (x >= OUTPUT_START_X && x <= OUTPUT_END_X) {
                    HoveringSlot = OUTPUT_SLOT;
                    Upload_Data(Buffers["Hover"].second, Get_Rect(OUTPUT_START_X, OUTPUT_END_X, OUTPUT_START_Y, OUTPUT_END_Y));
                }
            }
        }
    }
    
    if (HoldingStack.first) {
        Text::Set_Group(TEXT_GROUP);
        Text::Add("holdingStack", std::to_string(HoldingStack.second), mouseY + TEXT_PAD);
        Text::Set_X("holdingStack", mouseX + TEXT_PAD);
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
                Text::Add(textName, std::to_string(stack.second), startY + TEXT_PAD);
                Text::Set_X(textName, startX + TEXT_PAD);
            }
            
            else {
                Text::Set_Group(TOOLBAR_TEXT);
                Text::Add(textName, std::to_string(stack.second), TOOLBAR_START_Y + TEXT_PAD);
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
        index = 0;
        
        for (auto const &stack : Craft) {
            if (stack.first) {
                float startX = CRAFTING_START_X + (index % 3) * SLOT_WIDTH + SLOT_PAD;
                float startY = CRAFTING_START_Y + (index / 3) * SLOT_WIDTH + SLOT_PAD;
                
                Extend(data, Get_Vertices(stack.first, startX, startY, SLOT_WIDTH - SLOT_PAD * 2.0f));
                
                std::string textName = std::to_string(SLOTS_X * SLOTS_Y + index);
                
                Text::Set_Group(TEXT_GROUP);
                Text::Add(textName, std::to_string(stack.second), startY + TEXT_PAD);
                Text::Set_X(textName, startX + TEXT_PAD);
                Text::Unset_Group();
            }
            
            index++;
        }
        
        if (CraftingOutput.first) {
            Extend(data, Get_Vertices(CraftingOutput.first, OUTPUT_START_X + SLOT_PAD, OUTPUT_START_Y + SLOT_PAD, SLOT_WIDTH - SLOT_PAD * 2.0f));
            
            std::string textName = std::to_string(OUTPUT_SLOT);
            
            Text::Set_Group(TEXT_GROUP);
            Text::Add(textName, std::to_string(CraftingOutput.second), OUTPUT_START_Y + SLOT_PAD + TEXT_PAD);
            Text::Set_X(textName, OUTPUT_START_X + SLOT_PAD + TEXT_PAD);
            Text::Unset_Group();
        }
        
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
        glDrawArrays(GL_TRIANGLES, 0, 18);
        glBindVertexArray(0);
        
        glClear(GL_DEPTH_BUFFER_BIT);
        
        UIBorderShader->Bind();
        
        glBindVertexArray(Buffers["Grid"].first);
        
        glUniform3f(borderColorLocation, BORDER_COLOR.r, BORDER_COLOR.g, BORDER_COLOR.b);
        glDrawArrays(GL_LINES, 0, 52);
        
        glUniform3f(borderColorLocation, TOOLBAR_COLOR.r, TOOLBAR_COLOR.g, TOOLBAR_COLOR.b);
        glDrawArrays(GL_LINES, 52, 34);
        
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