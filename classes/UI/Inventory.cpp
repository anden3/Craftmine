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

const int OUTPUT_SLOT = INV_SIZE + 9;

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
    
    interface.Set_Document("toolbar");
    
    for (int i = 0; i < SLOTS_X; i++) {
        std::string name = std::to_string(i);
        float startX = TOOLBAR_START_X + i * (SLOT_WIDTH_X / 2.0f);
        float startY = TOOLBAR_START_Y;
        
        interface.Add_Text(name, "0", floor(startX + TEXT_PAD_X / 2.0f), floor(startY + TEXT_PAD_Y / 2.0f));
        interface.Get_Text_Element(name)->Opacity = 0.0f;
        interface.Add_3D_Element(name, 0, startX, startY + SLOT_PAD_Y, SLOT_WIDTH_X / 2.0f);
    }
    
    interface.Set_Document("inventory");
    
    for (int i = 0; i < INV_SIZE; i++) {
        float startX = START_X + (i % SLOTS_X) * SLOT_WIDTH_X;
        float startY = START_Y + (i / SLOTS_X) * SLOT_WIDTH_Y;
        std::string name = std::to_string(i);
        
        Inv.push_back(Stack());
        interface.Add_Text(name, "0", floor(startX + SLOT_PAD_X), floor(startY + SLOT_PAD_Y));
        interface.Get_Text_Element(name)->Opacity = 0.0f;
        interface.Add_3D_Element(name, 0, startX, startY + SLOT_PAD_Y, SLOT_WIDTH_X);
    }
    
    for (int i = 0; i < 9; i++) {
        float startX = CRAFTING_START_X + (i % 3) * SLOT_WIDTH_X + SLOT_PAD_X;
        float startY = CRAFTING_START_Y + (i / 3) * SLOT_WIDTH_Y + SLOT_PAD_Y;
        std::string name = std::to_string(INV_SIZE + i);
        
        Craft.push_back(Stack());
        interface.Add_Text(name, "0", startX, startY);
        interface.Get_Text_Element(name)->Opacity = 0.0f;
        interface.Add_3D_Element(name, 0, startX, startY, SLOT_WIDTH_X);
    }
    
    interface.Add_Text(std::to_string(INV_SIZE + 9), "0", OUTPUT_START_X + SLOT_PAD_X, OUTPUT_START_Y + SLOT_PAD_Y);
    interface.Get_Text_Element(std::to_string(INV_SIZE + 9))->Opacity = 0.0f;
    interface.Add_3D_Element(std::to_string(INV_SIZE + 9), 0, OUTPUT_START_X + SLOT_PAD_X, OUTPUT_START_Y + SLOT_PAD_Y, SLOT_WIDTH_X);
    
    interface.Add_Background("bgInv",
                             START_X - INV_PAD_X, START_Y - INV_PAD_Y,
                             (END_X - START_X) + INV_PAD_X * 2.0f, (END_Y - START_Y) + INV_PAD_Y * 2.0f, true, glm::vec2(SLOT_WIDTH_X, SLOT_WIDTH_Y));
    interface.Add_Background("bgCraft",
                             CRAFTING_START_X - INV_PAD_X, CRAFTING_START_Y - INV_PAD_Y,
                             (CRAFTING_END_X - CRAFTING_START_X) + INV_PAD_X * 2.0f, (CRAFTING_END_Y - CRAFTING_START_Y) + INV_PAD_Y * 2.0f, true, glm::vec2(SLOT_WIDTH_X, SLOT_WIDTH_Y));
    interface.Add_Background("bgOutput", OUTPUT_START_X - INV_PAD_X, OUTPUT_START_Y - INV_PAD_Y,
                             SLOT_WIDTH_X + INV_PAD_X * 2.0f, SLOT_WIDTH_Y + INV_PAD_Y * 2.0f, true);
    interface.Add_Background("bgHover", 0, 0, SLOT_WIDTH_X, SLOT_WIDTH_Y);
    
    interface.Add_Text("mouseStack", std::to_string(HoldingStack.Size), 0, 0);
    interface.Get_Text_Element("mouseStack")->Opacity = 0.0f;
    
    interface.Add_3D_Element("mouseStack", 0, 0, 0, SLOT_WIDTH_X);
    
    interface.Set_Document("toolbar");
    
    interface.Add_Background("bgToolbar", TOOLBAR_START_X, TOOLBAR_START_Y, (TOOLBAR_END_X - TOOLBAR_START_X), (TOOLBAR_END_Y - TOOLBAR_START_Y), true, glm::vec2(SLOT_WIDTH_X / 2.0f, SLOT_WIDTH_Y / 2.0f));
    interface.Add_Background("selectToolbar", TOOLBAR_START_X, TOOLBAR_START_Y, SLOT_WIDTH_X / 2.0f, (TOOLBAR_END_Y - TOOLBAR_START_Y), true, glm::vec2(SLOT_WIDTH_X / 2.0f, SLOT_WIDTH_Y / 2.0f));
    
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
    interface.Get_Background("selectToolbar")->Move(startX, TOOLBAR_START_Y);
}

void Inventory::Click_Handler(double x, double y, int button, int action) {
    if (HoveringSlot == -1) {
        return;
    }
    
    MouseDown = ((button == GLFW_MOUSE_BUTTON_LEFT) ? MOUSE_LEFT : MOUSE_RIGHT) * (action == GLFW_PRESS);
    
    if (HoveringSlot >= 0 && MouseDown) {
        MouseState = (HoldingStack.Type > 0);
        
        if (MouseDown == MOUSE_RIGHT) {
            Click_Slot(HoveringSlot, button);
            Mesh();
        }
        else {
            if (MouseState && HoveringSlot == OUTPUT_SLOT) {
                return;
            }
            
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
    interface.Set_Document("inventory");
    
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
            interface.Get_Background("bgHover")->Move(startX, startY, true);
        }
        
        else if (x >= CRAFTING_START_X && x <= CRAFTING_END_X) {
            if (mouseY >= CRAFTING_START_Y && mouseY <= CRAFTING_END_Y) {
                float startX = float(floor((x - CRAFTING_START_X) / SLOT_WIDTH_X) * SLOT_WIDTH_X + CRAFTING_START_X);
                float startY = float(floor((mouseY - CRAFTING_START_Y) / SLOT_WIDTH_Y) * SLOT_WIDTH_Y + CRAFTING_START_Y);
                
                HoveringSlot = INV_SIZE + int((startY - CRAFTING_START_Y) / SLOT_WIDTH_Y) * 3 + int((startX - CRAFTING_START_X) / SLOT_WIDTH_X);
                interface.Get_Background("bgHover")->Move(startX, startY, true);
            }
            
            else if (mouseY >= OUTPUT_START_Y && mouseY <= OUTPUT_END_Y) {
                if (x >= OUTPUT_START_X && x <= OUTPUT_END_X) {
                    HoveringSlot = OUTPUT_SLOT;
                    interface.Get_Background("bgHover")->Move(OUTPUT_START_X, OUTPUT_START_Y, true);
                }
            }
        }
    }
    
    if (HoveringSlot == -1) {
        interface.Get_Background("bgHover")->Opacity = 0.0f;
    }
    else {
        interface.Get_Background("bgHover")->Opacity = 0.5f;
    }
    
    if (HoveringSlot != LastSlot && HoveringSlot != -1) {
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
        TextElement* mouseStack = interface.Get_Text_Element("mouseStack");
        mouseStack->Text = std::to_string(HoldingStack.Size);
        mouseStack->Opacity = 1.0f;
        mouseStack->X = floor(mouseX + TEXT_PAD_X);
        mouseStack->Y = floor(mouseY + TEXT_PAD_Y);
        
        interface.Get_3D_Element("mouseStack")->Mesh(HoldingStack.Type, mouseX, mouseY);
    }
    else {
        interface.Get_3D_Element("mouseStack")->Mesh(0, mouseX, mouseY);
        interface.Get_Text_Element("mouseStack")->Opacity = 0.0f;
    }
    
    interface.Set_Document("");
}

void Inventory::Mesh() {
    int index = 0;
    
    for (auto const &stack : Inv) {
        float startX, startY;
        
        if (Is_Open) {
            interface.Set_Document("inventory");
            startX = START_X + (index % SLOTS_X) * SLOT_WIDTH_X + SLOT_PAD_X;
            startY = START_Y + (index / SLOTS_X) * SLOT_WIDTH_Y + SLOT_PAD_Y;
        }
        else {
            interface.Set_Document("toolbar");
            startX = TOOLBAR_START_X + (index % SLOTS_X) * SLOT_WIDTH_X / 2.0f + SLOT_PAD_X / 2.0f;
            startY = TOOLBAR_START_Y;
        }
        
        std::string textName = std::to_string(index);
        
        if (stack.Type) {
            interface.Get_3D_Element(textName)->Mesh(stack.Type, startX, startY + 10);
            interface.Get_Text_Element(textName)->Text = std::to_string(stack.Size);
            interface.Get_Text_Element(textName)->Opacity = 1.0f;
        }
        else {
            interface.Get_3D_Element(textName)->Type = 0;
            interface.Get_Text_Element(textName)->Opacity = 0.0f;
        }
        
        if (!Is_Open && index == 9) {
            break;
        }
        
        index++;
    }
    
    if (Is_Open) {
        interface.Set_Document("inventory");
        
        index = 0;
        
        for (auto const &stack : Craft) {
            std::string textName = std::to_string(INV_SIZE + index);
            
            if (stack.Type) {
                float startX = CRAFTING_START_X + (index % 3) * SLOT_WIDTH_X + SLOT_PAD_X;
                float startY = CRAFTING_START_Y + ((index / 3)) * SLOT_WIDTH_Y + SLOT_PAD_Y;
                
                interface.Get_3D_Element(textName)->Mesh(stack.Type, startX, startY + 10);
                interface.Get_Text_Element(textName)->Text = std::to_string(stack.Size);
                interface.Get_Text_Element(textName)->Opacity = 1.0f;
            }
            else {
                interface.Get_3D_Element(textName)->Type = 0;
                interface.Get_Text_Element(textName)->Opacity = 0.0f;
            }
            
            index++;
        }
        
        std::string outputName = std::to_string(OUTPUT_SLOT);
        
        if (CraftingOutput.Type) {
            interface.Get_3D_Element(outputName)->Mesh(CraftingOutput.Type, OUTPUT_START_X, OUTPUT_START_Y + INV_PAD_Y);
            interface.Get_Text_Element(outputName)->Text = std::to_string(CraftingOutput.Size);
            interface.Get_Text_Element(outputName)->Opacity = 1.0f;
        }
        else {
            interface.Get_3D_Element(outputName)->Type = 0;
            interface.Get_Text_Element(outputName)->Opacity = 0.0f;
        }
    }
    
    interface.Set_Document("");
}

void Inventory::Draw() {
    if (Is_Open) {
        interface.Draw_Document("inventory");
    }
    else {
        interface.Draw_Document("toolbar");
    }
}