#include "Inventory.h"

#include <glm/gtc/matrix_transform.hpp>

#include <regex>
#include <set>

const int SLOTS_X = 10;
const int SLOTS_Y = 6;

const int INV_SIZE = SLOTS_X * SLOTS_Y;
const int MAX_STACK_SIZE = 64;

float START_X, END_X;
float START_Y, END_Y;

float TOOLBAR_START_X, TOOLBAR_END_X;
float TOOLBAR_START_Y, TOOLBAR_END_Y;

float CRAFTING_START_X, CRAFTING_END_X;
float CRAFTING_START_Y, CRAFTING_END_Y;

float OUTPUT_START_X, OUTPUT_END_X;
float OUTPUT_START_Y, OUTPUT_END_Y;

float SLOT_WIDTH_X, SLOT_WIDTH_Y;
float INV_PAD_X, INV_PAD_Y;
float SLOT_PAD_X, SLOT_PAD_Y;
float TEXT_PAD_X, TEXT_PAD_Y;

float BLOCK_SCALE = 0.1f;

const int OUTPUT_SLOT = INV_SIZE + 9;

const glm::vec3 BACKGROUND_COLOR = glm::vec3(0.0f);
const glm::vec3 BORDER_COLOR = glm::vec3(0.5f);
const glm::vec3 TOOLBAR_COLOR = glm::vec3(1.0f);

const float BACKGROUND_OPACITY = 0.7f;

const std::string TEXT_GROUP = "inv";
const std::string TOOLBAR_TEXT = "toolbar";

enum MouseButtons {
    NONE,
    MOUSE_LEFT,
    MOUSE_RIGHT
};

static int MouseDown = NONE;

bool MouseState = 0;

int LastSlot = -1;
int StartSlot = -1;

std::set<Stack*> DivideSlots;
int HoldingSize = 0;

Shader* UIShader;
Shader* UIBorderShader;
Shader* UITextureShader;
Shader* UIOrthoShader;

class Recipe {
public:
    std::regex Pattern;
    Stack Result;
    
    Recipe(std::string pattern, Stack result) {
        Result = result;
        
        std::vector<std::string> setNums;
        
        std::string rgx = "";
        std::string setNum = "";
        std::string capture = "";
        
        bool set = false;
        bool capturing = false;
        bool checkNext = false;
        bool setAcceptZero = false;
        bool ignoreNextSpace = false;
        
        for (const char &c : pattern) {
            if (c == '(') {
                capture = "";
                capturing = true;
                continue;
            }
            
            if (capturing) {
                if (c == ')') {
                    capturing = false;
                    checkNext = true;
                }
                else {
                    if (c == ' ') {
                        capture += ",";
                    }
                    else {
                        capture += c;
                    }
                }
                
                continue;
            }
            
            if (c == '%') {
                rgx += "(0,)*";
                ignoreNextSpace = true;
            }
            
            else if (checkNext) {
                if (isdigit(c)) {
                    for (int i = 0; i < (c - 48); i++) {
                        rgx += capture + ",";
                    }
                    
                    rgx.pop_back();
                }
                
                else if (c == '[') {
                    set = true;
                }
                
                checkNext = false;
            }
            
            else if (set) {
                if (c == ']') {
                    set = false;
                    
                    setNums.push_back(setNum);
                    setNum = "";
                    
                    if (setAcceptZero) {
                        rgx += "(";
                    }
                    
                    for (auto const &num : setNums) {
                        rgx += "(" + capture + ",){" + num + "}|";
                    }
                    
                    rgx.pop_back();
                    
                    if (setAcceptZero) {
                        rgx += ")?";
                    }
                    
                    ignoreNextSpace = true;
                    setAcceptZero = false;
                }
                
                else if (c == '0') {
                    setAcceptZero = true;
                }
                
                else if (isdigit(c)) {
                    setNum += c;
                }
                
                else if (c == ',' && setNum != "") {
                    setNums.push_back(setNum);
                    setNum = "";
                }
            }
            
            else {
                if (c != ' ' ) {
                    rgx += c;
                }
                else if (!ignoreNextSpace) {
                    rgx += ",";
                }
                
                ignoreNextSpace = false;
            }
        }
        
        if (rgx.back() != ',' && rgx.back() != '*') {
            rgx += ",";
        }
        
        Pattern = std::regex("^" + rgx + "$");
    }
    
    inline bool Check(const std::string grid) const {
        return std::regex_match(grid.cbegin(), grid.cend(), Pattern);
    }
};

std::map<int, std::vector<Recipe>> Recipes = {
    // %   == Any number of zeroes.
    // ()  == Capture expression
    // ()x == Repeating captured expression x times.
    // []  == Repeating n times, where n is any of the numbers in the set.
    
    {1, std::vector<Recipe> {
        Recipe("% 17 %", Stack(5, 4)), // Wooden Planks
    }},
    
    {2, std::vector<Recipe> {
        Recipe("% 5 (0)2 5 %",     Stack(280, 4)), // Stick
        Recipe("% 280 (0)2 4 %",   Stack(69)),     // Lever
        Recipe("% 263 (0)2 280 %", Stack(50, 4)),  // Torch
    }},
    
    {3, std::vector<Recipe> {
        Recipe("% 265 (0 0 280)2 %", Stack(256)),   // Iron Shovel
        Recipe("(0)[0,3,6] (1)3 %",  Stack(44, 6)), // Stone Slab
    }},
    
    {4, std::vector<Recipe> {
        Recipe("% (5)2 0 (5)2 %",   Stack(58)), // Crafting Table
        Recipe("% (12)2 0 (12)2 %", Stack(24)), // Sandstone
    }},
    
    {5, std::vector<Recipe> {
        Recipe("% (265)2 0 265 280 (0)2 280 %", Stack(258)), // Iron Axe
        Recipe("(265)3 (0 280 0)2",             Stack(257)), // Iron Pickaxe
    }},
    
    {6, std::vector<Recipe> {
        Recipe("(0)[0,3] (35)3 (5)3 %", Stack(26)), // Bed
    }},
    
    {7, std::vector<Recipe> {
        Recipe("265 0 (265)2 280 (265)2 0 265", Stack(66, 16)), // Rail
        Recipe("280 0 (280)5 0 280",            Stack(65, 4)), // Ladder
    }},
    
    {8, std::vector<Recipe> {
        Recipe("(4)4 0 (4)4", Stack(61)), // Furnace
        Recipe("(5)4 0 (5)4", Stack(54)), // Chest
    }},
    
    {9, std::vector<Recipe> {
        Recipe("(264)9", Stack(57)), // Diamond Block
        Recipe("(265)9", Stack(42)), // Iron Block
        Recipe("(266)9", Stack(41)), // Gold Block
    }},
};

Data Get_Rect(float x1, float x2, float y1, float y2) {
    return Data {x1, y1, x2, y1, x2, y2, x1, y1, x2, y2, x1, y2};
}

Data Get_Vertices(int type, float baseX, float baseY, float multiplierX, float multiplierY) {
    if (multiplierY == -1) {
        multiplierY = multiplierX;
    }
    
    static float textureStepX = (1.0f / 16.0f);
    static float textureStepY = (1.0f / 32.0f);
    static float vertices[6][2] = { {0, 0}, {1, 0}, {1, 1}, {0, 0}, {1, 1}, {0, 1} };
    static float texCoords[6][2] = { {0, 1}, {1, 1}, {1, 0}, {0, 1}, {1, 0}, {0, 0} };
    
    glm::vec2 texPosition = textureCoords[type];
    
    Data result;
    
    for (int i = 0; i < 6; i++) {
        result.push_back(baseX + vertices[i][0] * multiplierX);
        result.push_back(baseY + vertices[i][1] * multiplierY);
        
        result.push_back((texPosition.x + texCoords[i][0] - 1.0f) * textureStepX);
        result.push_back((texPosition.y + texCoords[i][1] - 1.0f) * textureStepY);
    }
    
    return result;
}

void Inventory::Init() {
    UIOrthoShader = new Shader("ortho");
    
    START_X = X_Frac(11, 72);
    END_X = X_Frac(17, 24);
    
    START_Y = Y_Frac(11, 45);
    END_Y = Y_Frac(7, 9);
    
    TOOLBAR_START_X = X_Frac(13, 36);
    TOOLBAR_END_X = X_Frac(23, 36);
    
    TOOLBAR_START_Y = Y_Frac(2, 45);
    TOOLBAR_END_Y = Y_Frac(4, 45);
    
    CRAFTING_START_X = X_Frac(55, 72);
    CRAFTING_END_X = X_Frac(67, 72);
    
    CRAFTING_START_Y = Y_Frac(23, 45);
    CRAFTING_END_Y = Y_Frac(7, 9);
    
    SLOT_WIDTH_X = X_Frac(1, 18);
    SLOT_WIDTH_Y = Y_Frac(4, 45);
    
    OUTPUT_START_X = CRAFTING_START_X + SLOT_WIDTH_X;
    OUTPUT_END_X = CRAFTING_END_X - SLOT_WIDTH_X;
    
    OUTPUT_START_Y = CRAFTING_START_Y - SLOT_WIDTH_Y * 2;
    OUTPUT_END_Y = CRAFTING_START_Y - SLOT_WIDTH_Y;
    
    INV_PAD_X = X_Frac(1, 144);
    INV_PAD_Y = Y_Frac(1, 90);
    
    SLOT_PAD_X = X_Frac(1, 144);
    SLOT_PAD_Y = Y_Frac(1, 90);
    
    TEXT_PAD_X = X_Frac(1, 288);
    TEXT_PAD_Y = Y_Frac(1, 180);
    
    for (int i = 0; i < INV_SIZE; i++) {
        Inv.push_back(Stack());
    }
    
    for (int i = 0; i < 9; i++) {
        Craft.push_back(Stack());
    }
    
    Init_UI();
}

void Inventory::Init_UI() {
    Data gridData;
    Data toolbarGridData;
    
    // Fix for missing pixel in upper left corner
    Extend(gridData, Data {START_X - 0.5f, END_Y + 0.5f, START_X + 0.5f, END_Y});
    Extend(gridData, Data {CRAFTING_START_X - 0.5f, CRAFTING_END_Y + 0.5f, CRAFTING_START_X + 0.5f, CRAFTING_END_Y});
    
    // Grey vertical lines
    for (float x = START_X; x <= END_X; x += SLOT_WIDTH_X) {
        Extend(gridData, Data {x, START_Y + SLOT_WIDTH_Y + 1.5f, x, END_Y});
    }
    
    for (float x = CRAFTING_START_X; x <= CRAFTING_END_X; x += SLOT_WIDTH_X) {
        Extend(gridData, Data {x, CRAFTING_START_Y, x, CRAFTING_END_Y});
    }
    
    // Grey horizontal lines
    for (float y = START_Y + SLOT_WIDTH_Y * 2; y <= END_Y; y += SLOT_WIDTH_Y) {
        Extend(gridData, Data {START_X, y, END_X, y});
    }
    
    for (float y = CRAFTING_START_Y; y <= CRAFTING_END_Y; y += SLOT_WIDTH_Y) {
        Extend(gridData, Data {CRAFTING_START_X, y, CRAFTING_END_X, y});
    }
    
    // White horizontal lines
    Extend(gridData, Data {START_X - 0.5f, START_Y + SLOT_WIDTH_Y, END_X, START_Y + SLOT_WIDTH_Y, START_X, START_Y, END_X, START_Y});
    
    // White vertical lines
    for (float x = START_X; x <= END_X; x += SLOT_WIDTH_X) {
        Extend(gridData, Data {x, START_Y, x, START_Y + SLOT_WIDTH_Y});
    }
    
    // Crafting output
    Extend(gridData, Data {OUTPUT_START_X - 0.5f, OUTPUT_END_Y,   OUTPUT_END_X, OUTPUT_END_Y});
    Extend(gridData, Data {OUTPUT_START_X - 0.5f, OUTPUT_START_Y, OUTPUT_END_X, OUTPUT_START_Y});
    
    Extend(gridData, Data {OUTPUT_START_X - 0.5f, OUTPUT_END_Y, OUTPUT_START_X - 0.5f, OUTPUT_START_Y});
    Extend(gridData, Data {OUTPUT_END_X, OUTPUT_END_Y, OUTPUT_END_X, OUTPUT_START_Y});
    
    Extend(toolbarGridData, Data {
        TOOLBAR_START_X - 0.5f, TOOLBAR_END_Y, TOOLBAR_END_X, TOOLBAR_END_Y, TOOLBAR_START_X, TOOLBAR_START_Y, TOOLBAR_END_X, TOOLBAR_START_Y
    });
    
    for (float x = TOOLBAR_START_X; x <= TOOLBAR_END_X; x += SLOT_WIDTH_X / 2) {
        Extend(toolbarGridData, Data {x, TOOLBAR_END_Y, x, TOOLBAR_START_Y});
    }
    
    Data bgData = Get_Rect(START_X - INV_PAD_X, END_X + INV_PAD_X, START_Y - INV_PAD_Y, END_Y + INV_PAD_Y);
    Extend(bgData, Get_Rect(CRAFTING_START_X - INV_PAD_X, CRAFTING_END_X + INV_PAD_X, CRAFTING_START_Y - INV_PAD_Y, CRAFTING_END_Y + INV_PAD_Y));
    Extend(bgData, Get_Rect(OUTPUT_START_X - INV_PAD_X, OUTPUT_END_X + INV_PAD_X, OUTPUT_START_Y - INV_PAD_Y, OUTPUT_END_Y + INV_PAD_Y));
    
    Data data[4] = {
        bgData, gridData, toolbarGridData,
        Get_Rect(TOOLBAR_START_X, TOOLBAR_END_X, TOOLBAR_START_Y, TOOLBAR_END_Y) // Toolbar background
    };
    
    Shader* shaders[3] = {UIShader, UIBorderShader, UITextureShader};
    
    int index = 0;
    
    for (auto const &buffer : BufferNames) {
        Buffers[buffer.first] = Buffer();
        Buffers[buffer.first].Init(shaders[buffer.second]);
        
        std::vector<int> config = {2};
        
        if (index >= 6) {
            config.push_back(2);
        }
        
        if (index < 4) {
            Buffers[buffer.first].Create(config, data[index]);
        }
        else {
            Buffers[buffer.first].Create(config);
        }
        
        if (buffer.first == "ToolbarGrid" || buffer.first == "Grid") {
            Buffers[buffer.first].VertexType = GL_LINES;
        }
        
        index++;
    }
    
    Render_GUI_Block(2);
    Switch_Slot();
}

void Inventory::Clear() {
    for (int i = 0; i < int(Inv.size()); i++) {
        Inv[i] = Stack();
    }
    
    Mesh();
}

void Inventory::Add_Stack(unsigned int type, unsigned int size) {
    int index = 0;
    
    for (auto const &stack : Inv) {
        if (stack.Type == type) {
            if (stack.Size < MAX_STACK_SIZE) {
                if (stack.Size + size <= MAX_STACK_SIZE) {
                    Inv[index].Size += size;
                    Mesh();
                    return;
                }
                else {
                    size -= MAX_STACK_SIZE - stack.Size;
                    Inv[index].Size = MAX_STACK_SIZE;
                }
            }
        }
        
        index++;
    }
    
    index = 0;
    
    for (auto const &stack : Inv) {
        if (stack.Type == 0) {
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
    
    Inv[slot].Size--;
    
    if (Inv[slot].Size == 0) {
        Inv[slot].Type = 0;
    }
    
    Mesh();
}

Stack Inventory::Get_Info(int slot) {
    if (slot == -1) {
        slot = ActiveToolbarSlot;
    }
    
    return Inv[slot];
}

Stack& Inventory::Get_Stack(int slot) {
    return (slot >= INV_SIZE) ? Craft[slot - INV_SIZE] : Inv[slot];
}

void Inventory::Click_Slot(int slot, int button) {
    if (slot == OUTPUT_SLOT && CraftingOutput.Type) {
        if (keys[GLFW_KEY_LEFT_SHIFT]) {
            Add_Stack(CraftingOutput);
            CraftingOutput = Stack();
            Craft_Item();
        }
        
        else if (!HoldingStack.Type) {
            Swap_Stacks(CraftingOutput);
            Craft_Item();
        }
        
        else if (Left_Click_Stack(CraftingOutput, HoldingStack)) {
            Craft_Item();
        }
    }
    else {
        Stack& stack = Get_Stack(slot);
        
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            Left_Click_Stack(HoldingStack, stack, true);
        }
        else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            Right_Click_Stack(stack);
        }
    }
    
    if (slot >= INV_SIZE) {
        Check_Crafting();
    }
}

void Inventory::Swap_Stacks(Stack &stack) {
    Stack toBePlaced = HoldingStack;
    HoldingStack = stack;
    stack = toBePlaced;
}

bool Inventory::Left_Click_Stack(Stack &a, Stack &b, bool swap) {
    if (a.Type != 0 && a.Type == b.Type) {
        if (b.Size < MAX_STACK_SIZE) {
            if (a.Size <= MAX_STACK_SIZE - b.Size) {
                b.Size += a.Size;
                a = Stack();
            }
            else {
                int remainder = a.Size - (MAX_STACK_SIZE - b.Size);
                b.Size = MAX_STACK_SIZE;
                a.Size = remainder;
            }
            
            return true;
        }
    }
    
    if (swap) {
        Swap_Stacks(b);
    }
    
    return false;
}

void Inventory::Left_Drag(int slot) {
    Stack &stack = (slot >= INV_SIZE) ? Craft[slot - INV_SIZE] : Inv[slot];
    
    if (MouseState == 0) {
        if (HoldingStack.Type == 0 || stack.Type == HoldingStack.Type) {
            HoldingStack.Type = stack.Type;
            
            if (HoldingStack.Size <= MAX_STACK_SIZE - stack.Size) {
                HoldingStack.Size += stack.Size;
                stack = Stack();
            }
            else {
                int remainder = stack.Size - (MAX_STACK_SIZE - HoldingStack.Size);
                HoldingStack.Size = MAX_STACK_SIZE;
                stack.Size = remainder;
                
                if (stack.Size == 0) {
                    stack.Type = 0;
                }
            }
            
            Mesh();
        }
    }
    else if (!stack.Type || stack.Type == HoldingStack.Type) {
        if (HoldingSize <= DivideSlots.size()) {
            return;
        }
        
        stack.Type = HoldingStack.Type;
        DivideSlots.insert(&stack);
        
        if (DivideSlots.size() > 1) {
            int floorNum = HoldingSize / int(DivideSlots.size());
            int remainder = HoldingSize % int(DivideSlots.size());
            
            for (auto &stack : DivideSlots) {
                stack->Size += floorNum;
            }
            
            HoldingStack.Size = remainder;
            
            Mesh();
            
            for (auto &stack : DivideSlots) {
                stack->Size -= floorNum;
            }
        }
    }
}

void Inventory::Right_Click_Stack(Stack &stack) {
    if (HoldingStack.Type) {
        if (HoldingStack.Type == stack.Type && stack.Size < MAX_STACK_SIZE) {
            stack.Size++;
        }
        else if (!stack.Type) {
            stack = Stack(HoldingStack.Type);
        }
        
        if (--HoldingStack.Size == 0) {
            HoldingStack.Type = 0;
        }
    }
    else if (stack.Type) {
        HoldingStack.Type = stack.Type;
        HoldingStack.Size = int(ceil(stack.Size / 2.0));
        stack.Size -= HoldingStack.Size;
        
        if (stack.Size == 0) {
            stack.Type = 0;
        }
    }
}

void Inventory::Check_Crafting() {
    std::string grid = "";
    int blocks = 0;
    
    for (auto const &tile : Craft) {
        grid += std::to_string(tile.Type) + ",";
        blocks += tile.Type > 0;
    }
    
    for (auto const &recipe : Recipes[blocks]) {
        if (recipe.Check(grid)) {
            CraftingOutput = recipe.Result;
            Mesh();
            return;
        }
    }
    
    CraftingOutput = Stack();
    Mesh();
}

void Inventory::Craft_Item() {
    for (auto &stack : Craft) {
        if (stack.Type) {
            if (--stack.Size == 0) {
                stack.Type = 0;
            }
        }
    }
}

void Inventory::Switch_Slot() {
    float startX = TOOLBAR_START_X + ActiveToolbarSlot * SLOT_WIDTH_X / 2.0f;
    Buffers["ToolbarSelect"].Upload(Get_Rect(startX, startX + SLOT_WIDTH_X / 2.0f, TOOLBAR_START_Y, TOOLBAR_END_Y));
}

void Inventory::Click_Handler(double x, double y, int button, int action) {
    MouseDown = ((button == GLFW_MOUSE_BUTTON_LEFT) ? MOUSE_LEFT : MOUSE_RIGHT) * (action == GLFW_PRESS);
    
    if (HoveringSlot >= 0 && MouseDown) {
        MouseState = (HoldingStack.Type > 0);
        
        if (MouseDown == MOUSE_RIGHT) {
            Click_Slot(HoveringSlot, button);
            Mesh();
        }
        else {
            StartSlot = HoveringSlot;
            
            if (MouseState) {
                HoldingSize = HoldingStack.Size;
                Left_Drag(HoveringSlot);
            }
        }
    }
    
    else if (!MouseDown) {
        if (StartSlot == HoveringSlot && DivideSlots.size() < 2) {
            StartSlot = -1;
            Click_Slot(HoveringSlot, GLFW_MOUSE_BUTTON_LEFT);
        }
        
        else if (!DivideSlots.empty()) {
            int floorNum = HoldingSize / int(DivideSlots.size());
            int remainder = HoldingSize % int(DivideSlots.size());
            
            for (auto &stack : DivideSlots) {
                stack->Type = HoldingStack.Type;
                stack->Size += floorNum;
            }
            
            HoldingStack.Size = remainder;
            
            if (remainder == 0) {
                HoldingStack = Stack();
            }
        }
        
        DivideSlots.clear();
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
            float startX = float(floor((x - START_X) / SLOT_WIDTH_X) * SLOT_WIDTH_X + START_X);
            float startY = float(floor((900 - y - START_Y) / SLOT_WIDTH_Y) * SLOT_WIDTH_Y + START_Y);
            
            HoveringSlot = int((startY - START_Y) / SLOT_WIDTH_Y) * SLOTS_X + int((startX - START_X) / SLOT_WIDTH_X);
            
            Buffers["Hover"].Upload(Get_Rect(startX, startX + SLOT_WIDTH_X, startY, startY + SLOT_WIDTH_Y));
        }
        
        else if (x >= CRAFTING_START_X && x <= CRAFTING_END_X) {
            if (mouseY >= CRAFTING_START_Y && mouseY <= CRAFTING_END_Y) {
                float startX = float(floor((x - CRAFTING_START_X) / SLOT_WIDTH_X) * SLOT_WIDTH_X + CRAFTING_START_X);
                float startY = float(floor((mouseY - CRAFTING_START_Y) / SLOT_WIDTH_Y) * SLOT_WIDTH_Y + CRAFTING_START_Y);
                
                HoveringSlot = INV_SIZE + int(2 - (startY - CRAFTING_START_Y) / SLOT_WIDTH_Y) * 3 + int((startX - CRAFTING_START_X) / SLOT_WIDTH_X);
                
                Buffers["Hover"].Upload(Get_Rect(startX, startX + SLOT_WIDTH_X, startY, startY + SLOT_WIDTH_Y));
            }
            
            else if (mouseY >= OUTPUT_START_Y && mouseY <= OUTPUT_END_Y) {
                if (x >= OUTPUT_START_X && x <= OUTPUT_END_X) {
                    HoveringSlot = OUTPUT_SLOT;
                    Buffers["Hover"].Upload(Get_Rect(OUTPUT_START_X, OUTPUT_END_X, OUTPUT_START_Y, OUTPUT_END_Y));
                }
            }
        }
    }
    
    if (HoveringSlot != LastSlot) {
        LastSlot = HoveringSlot;
        
        if (MouseDown == MOUSE_RIGHT && MouseState == 1) {
            Click_Slot(HoveringSlot, GLFW_MOUSE_BUTTON_RIGHT);
            Mesh();
        }
        else if (MouseDown == MOUSE_LEFT && HoveringSlot != OUTPUT_SLOT) {
            Left_Drag(HoveringSlot);
        }
    }
    
    if (HoldingStack.Type) {
        Text::Set_Group(TEXT_GROUP);
        Text::Add("holdingStack", std::to_string(HoldingStack.Size), mouseY + TEXT_PAD_Y);
        Text::Set_X("holdingStack", mouseX + TEXT_PAD_X);
        Text::Set_Opacity("holdingStack", 1.0f);
        Text::Unset_Group();
        
        Buffers["Mouse"].Upload(Get_Vertices(HoldingStack.Type, mouseX, mouseY, SLOT_WIDTH_X / 2.0f, SLOT_WIDTH_Y / 2.0f));
    }
}

void Inventory::Render_GUI_Block(unsigned int type) {
    glm::mat4 projection = glm::ortho(100, 200, 100, 200);
    glm::mat4 model;
    
    UIOrthoShader->Upload("projection", projection);
    UIOrthoShader->Upload("model", model);
    
    Data data;
    
    glm::vec2 texPosition = textureCoords[type];
    static float textureStepX = (1.0f / 16.0f);
    static float textureStepY = (1.0f / 32.0f);
    
    float texStartX = textureStepX * (texPosition.x - 1.0f);
    float texStartY = textureStepY * (texPosition.y - 1.0f);
    
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            if (CustomVertices.count(type)) {
                data.push_back((CustomVertices[type][i][vertices[i][j][0]].x - 0.5f) * BLOCK_SCALE);
                data.push_back((CustomVertices[type][i][vertices[i][j][1]].y - 0.5f) * BLOCK_SCALE);
                data.push_back((CustomVertices[type][i][vertices[i][j][2]].z - 0.5f) * BLOCK_SCALE);
            }
            else {
                data.push_back((vertices[i][j][0] - 0.5f) * BLOCK_SCALE);
                data.push_back((vertices[i][j][1] - 0.5f) * BLOCK_SCALE);
                data.push_back((vertices[i][j][2] - 0.5f) * BLOCK_SCALE);
            }
            
            if (CustomTexCoords.count(type)) {
                data.push_back(CustomTexCoords[type][i][tex_coords[i][j][0]].x / 16.0f);
                data.push_back(CustomTexCoords[type][i][tex_coords[i][j][1]].y / 32.0f);
            }
            else if (MultiTextures.count(type)) {
                data.push_back((MultiTextures[type][i].x - 1.0f + tex_coords[i][j][0]) / 16.0f);
                data.push_back((MultiTextures[type][i].y - 1.0f + tex_coords[i][j][1]) / 32.0f);
            }
            else {
                data.push_back(texStartX + tex_coords[i][j][0] / 16.0f);
                data.push_back(texStartY + tex_coords[i][j][1] / 32.0f);
            }
        }
    }
    
    TestBuffer.Init(UIOrthoShader);
    TestBuffer.Create(std::vector<int>{3, 2}, data);
}

void Inventory::Mesh() {
    Text::Delete_Group(TEXT_GROUP);
    Text::Delete_Group(TOOLBAR_TEXT);
    
    Data data;
    Data toolbarData;
    
    int index = 0;
    
    for (auto const &stack : Inv) {
        if (stack.Type) {
            float startX = START_X + (index % SLOTS_X) * SLOT_WIDTH_X + SLOT_PAD_X;
			float startY = START_Y + (index / SLOTS_X) * SLOT_WIDTH_Y + SLOT_PAD_Y;
            
			float toolbarStartX = TOOLBAR_START_X + (index % SLOTS_X) * SLOT_WIDTH_X / 2.0f + SLOT_PAD_X / 2.0f;
            
            if (Is_Open) {
                Extend(data, Get_Vertices(stack.Type, startX, startY, SLOT_WIDTH_X - SLOT_PAD_X * 2.0f, SLOT_WIDTH_Y - SLOT_PAD_Y * 2.0f));
            }
            else {
                Extend(toolbarData, Get_Vertices(stack.Type, toolbarStartX, TOOLBAR_START_Y + SLOT_PAD_Y / 2, SLOT_WIDTH_X / 2 - SLOT_PAD_X, SLOT_WIDTH_Y / 2 - SLOT_PAD_Y));
            }
            
            std::string textName = std::to_string(index);
            
            if (Is_Open) {
                Text::Set_Group(TEXT_GROUP);
                Text::Add(textName, std::to_string(stack.Size), startY + TEXT_PAD_Y);
                Text::Set_X(textName, startX + TEXT_PAD_X);
            }
            
            else {
                Text::Set_Group(TOOLBAR_TEXT);
                Text::Add(textName, std::to_string(stack.Size), TOOLBAR_START_Y + TEXT_PAD_Y);
                Text::Set_X(textName, toolbarStartX + TEXT_PAD_X / 2.0f);
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
            if (stack.Type) {
                float startX = CRAFTING_START_X + (index % 3) * SLOT_WIDTH_X + SLOT_PAD_X;
                float startY = CRAFTING_START_Y + (2 - (index / 3)) * SLOT_WIDTH_Y + SLOT_PAD_Y;
                
                Extend(data, Get_Vertices(stack.Type, startX, startY, SLOT_WIDTH_X - SLOT_PAD_X * 2.0f, SLOT_WIDTH_Y - SLOT_PAD_Y * 2.0f));
                
                std::string textName = std::to_string(INV_SIZE + index);
                
                Text::Set_Group(TEXT_GROUP);
                Text::Add(textName, std::to_string(stack.Size), startY + TEXT_PAD_Y);
                Text::Set_X(textName, startX + TEXT_PAD_X);
                Text::Unset_Group();
            }
            
            index++;
        }
        
        if (CraftingOutput.Type) {
            Extend(data, Get_Vertices(CraftingOutput.Type, OUTPUT_START_X + SLOT_PAD_X, OUTPUT_START_Y + SLOT_PAD_Y, SLOT_WIDTH_X - SLOT_PAD_X * 2.0f, SLOT_WIDTH_Y - SLOT_PAD_Y * 2.0f));
            
            std::string textName = std::to_string(OUTPUT_SLOT);
            
            Text::Set_Group(TEXT_GROUP);
            Text::Add(textName, std::to_string(CraftingOutput.Size), OUTPUT_START_Y + SLOT_PAD_Y + TEXT_PAD_Y);
            Text::Set_X(textName, OUTPUT_START_X + SLOT_PAD_X + TEXT_PAD_X);
            Text::Unset_Group();
        }
        
        Buffers["Slots"].Upload(data);
    }
    
    else {
        Buffers["Toolbar"].Upload(toolbarData);
    }
}

void Inventory::Draw() {
    if (Is_Open) {
        UIShader->Upload(colorLocation, BACKGROUND_COLOR);
        UIShader->Upload(alphaLocation, BACKGROUND_OPACITY);
        
        Buffers["Background"].Draw();
        
        glClear(GL_DEPTH_BUFFER_BIT);
        
        UIBorderShader->Upload(borderColorLocation, BORDER_COLOR);
        Buffers["Grid"].Draw(0, 52);
        
        UIBorderShader->Upload(borderColorLocation, TOOLBAR_COLOR);
        Buffers["Grid"].Draw(52, 34);
        
        Buffers["Slots"].Draw();
        
        if (HoldingStack.Type) {
            glClear(GL_DEPTH_BUFFER_BIT);
            Buffers["Mouse"].Draw();
        }
        
        glClear(GL_DEPTH_BUFFER_BIT);
        Text::Draw_Group(TEXT_GROUP);
        
        if (HoveringSlot >= 0) {
            glClear(GL_DEPTH_BUFFER_BIT);
            
            UIShader->Upload(colorLocation, glm::vec3(1));
            UIShader->Upload(alphaLocation, 0.3f);
            
            Buffers["Hover"].Draw();
        }
        
        glClear(GL_DEPTH_BUFFER_BIT);
        TestBuffer.Draw();
    }
    else {
        UIShader->Upload(alphaLocation, BACKGROUND_OPACITY);
        UIShader->Upload(colorLocation, BACKGROUND_COLOR);
        Buffers["ToolbarBackground"].Draw();
        
        glClear(GL_DEPTH_BUFFER_BIT);
        
        UIShader->Upload(colorLocation, glm::vec3(1));
        Buffers["ToolbarSelect"].Draw();
        
        UIBorderShader->Upload(borderColorLocation, TOOLBAR_COLOR);
        Buffers["ToolbarGrid"].Draw();
        
        glClear(GL_DEPTH_BUFFER_BIT);
        
        Buffers["Toolbar"].Draw();
        
        glClear(GL_DEPTH_BUFFER_BIT);
        Text::Draw_Group(TOOLBAR_TEXT);
    }
}