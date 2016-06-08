#include "Inventory.h"

#include <glm/gtc/matrix_transform.hpp>

#include <regex>
#include <set>

#include <GLFW/glfw3.h>

#include "main.h"
#include "Interface.h"

const int SLOTS_X = 10;
const int SLOTS_Y = 7;

const int INV_SIZE = SLOTS_X * SLOTS_Y;
const int MAX_STACK_SIZE = 64;

float BLOCK_SCALE;

glm::vec4 barDims;
glm::vec4 invDims;
glm::vec4 craftDims;
glm::vec4 outputDims;
glm::vec4 invBarDims;
glm::vec2 slotWidth;
glm::vec2 invPad;
glm::vec2 slotPad;
glm::vec2 textPad;

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
                capture.clear();
                capturing = true;
            }
            
            else if (c == '%') {
                rgx += "(0,)*";
                ignoreNextSpace = true;
            }
            
            else if (capturing) {
                if (c == ')') {
                    capturing = false;
                    checkNext = true;
                }
                else {
                    capture += (c == ' ') ? ',' : c;
                }
            }
            
            else if (checkNext) {
                checkNext = false;
                
                if (isdigit(c)) {
                    for (int i = 0; i < (c - 48); i++) {
                        rgx += capture + ",";
                    }
                    
                    rgx.pop_back();
                }
                
                else if (c == '[') {
                    set = true;
                }
            }
            
            else if (set) {
                if (c == ']') {
                    set = false;
                    
                    setNums.push_back(setNum);
                    setNum.clear();
                    
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
    // %   == Any number (including zero) of zeroes.
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
        Recipe("280 0 (280)5 0 280",            Stack(65, 4)),  // Ladder
    }},
    
    {8, std::vector<Recipe> {
        Recipe("(4)4 0 (4)4", Stack(61)), // Furnace
        Recipe("(5)4 0 (5)4", Stack(54)), // Chest
    }},
    
    {9, std::vector<Recipe> {
        Recipe("(264)9", Stack(57)), // Diamond Block
        Recipe("(265)9", Stack(42)), // Iron Block
        Recipe("(266)9", Stack(41)), // Gold Block
    }}
};

inline bool In_Range(float value, glm::vec2 bounds) {
    return value >= bounds.x && value <= (bounds.x + bounds.y);
}

void Inventory::Init() {
    barDims = glm::vec4(Scale(520, 40), Scale(400, 40));
    invDims = glm::vec4(Scale(220), Scale(800, 480));
    craftDims = glm::vec4(Scale(1100, 460), Scale(240));
    outputDims = glm::vec4(Scale(1180, 300), Scale(80));
    invBarDims = glm::vec4(Scale(220, 100), Scale(800, 80));
    
    slotWidth = Scale(80);
    invPad = Scale(10);
    slotPad = Scale(10);
    textPad = Scale(5);
    
    BLOCK_SCALE = Scale_X(80);
    
    interface.Set_Document("toolbar");
    
    for (int i = 0; i < SLOTS_X; i++) {
        std::string name = std::to_string(i);
        glm::vec2 pos(floor(barDims.x + i * (slotWidth.x / 2.0f)), floor(barDims.y));
        
        interface.Add_Text(name, "0", pos + textPad / 2.0f);
        interface.Get_Text_Element(name)->Opacity = 0.0f;
        interface.Add_3D_Element(name, 0, 0, pos + glm::vec2(0, slotPad.y), BLOCK_SCALE / 2.0f);
    }
    
    interface.Set_Document("inventory");
    
    for (int i = 0; i < INV_SIZE; i++) {
        std::string name = std::to_string(i);
        glm::vec2 pos(floor(invDims.x + (i % SLOTS_X) * slotWidth.x), floor((i < SLOTS_X) ? invBarDims.y : invDims.y + ((i / SLOTS_X) - 1) * slotWidth.y));
        
        Inv.push_back(Stack());
        interface.Add_Text(name, "0", pos + slotPad);
        interface.Get_Text_Element(name)->Opacity = 0.0f;
        interface.Add_3D_Element(name, 0, 0, pos + glm::vec2(0, slotPad.y), BLOCK_SCALE);
    }
    
    for (int i = 0; i < 9; i++) {
        std::string name = std::to_string(INV_SIZE + i);
        glm::vec2 pos = glm::vec2(i % 3, i / 3) * slotWidth + craftDims.xy() + slotPad;
        
        Craft.push_back(Stack());
        interface.Add_Text(name, "0", pos);
        interface.Get_Text_Element(name)->Opacity = 0.0f;
        interface.Add_3D_Element(name, 0, 0, pos, BLOCK_SCALE);
    }
    
    interface.Add_Text(std::to_string(INV_SIZE + 9), "0", outputDims.xy() + slotPad);
    interface.Get_Text_Element(std::to_string(INV_SIZE + 9))->Opacity = 0.0f;
    interface.Add_3D_Element(std::to_string(INV_SIZE + 9), 0, 0, outputDims.xy() + glm::vec2(slotPad.x, slotPad.y * 2), BLOCK_SCALE);
    
    interface.Add_Background("invInv", glm::vec4(invDims.xy() - invPad, invDims.zw() + invPad * 2.0f), true, slotWidth, invPad);
    interface.Add_Background("invCraft", glm::vec4(craftDims.xy() - invPad, craftDims.zw() + invPad * 2.0f), true, slotWidth, invPad);
    interface.Add_Background("invOutput", glm::vec4(outputDims.xy() - invPad, outputDims.zw() + invPad * 2.0f), true, slotWidth, invPad);
    interface.Add_Background("invToolbar", glm::vec4(invBarDims.xy() - invPad, invBarDims.zw() + invPad * 2.0f), true, slotWidth, invPad);
    
    interface.Add_Background("invHover", glm::vec4(0, 0, slotWidth));
    interface.Get_Background("invHover")->Color = glm::vec3(0.7f);
    
    interface.Add_Text("mouseStack", std::to_string(HoldingStack.Size), 0, 0);
    interface.Get_Text_Element("mouseStack")->Opacity = 0.0f;
    
    interface.Add_3D_Element("mouseStack", 0, 0, 0, 0, BLOCK_SCALE);
    
    interface.Set_Document("toolbar");
    
    interface.Add_Background("bgToolbar", barDims, true, slotWidth / 2.0f);
    interface.Add_Background("selectToolbar", glm::vec4(barDims.xy(), slotWidth.x / 2.0f, barDims.w), true, slotWidth / 2.0f);
    interface.Get_Background("selectToolbar")->Color = glm::vec3(0.7f);
    
    Switch_Slot();
}

void Inventory::Clear() {
    for (int i = 0; i < int(Inv.size()); i++) {
        Inv[i].Clear();
    }
    
    Mesh();
}

void Inventory::Add_Stack(int type, int typeData, int size) {
    int index = 0;
    
    for (auto &stack : Inv) {
        if (stack.Type == type && stack.Data == typeData) {
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
                Inv[index] = Stack(type, typeData, size);
                Mesh();
                return;
            }
            else {
                Inv[index] = Stack(type, typeData, MAX_STACK_SIZE);
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
        Inv[slot].Clear();
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
            CraftingOutput.Clear();
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
    if (a.Type != 0 && a.Type == b.Type && a.Data == b.Data) {
        if (b.Size < MAX_STACK_SIZE) {
            if (a.Size <= MAX_STACK_SIZE - b.Size) {
                b.Size += a.Size;
                a.Clear();
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
        if (HoldingStack.Type == 0 || (stack.Type == HoldingStack.Type && stack.Data == HoldingStack.Data)) {
            HoldingStack.Type = stack.Type;
            HoldingStack.Data = stack.Data;
            
            if (HoldingStack.Size <= MAX_STACK_SIZE - stack.Size) {
                HoldingStack.Size += stack.Size;
                stack.Clear();
            }
            else {
                int remainder = stack.Size - (MAX_STACK_SIZE - HoldingStack.Size);
                HoldingStack.Size = MAX_STACK_SIZE;
                stack.Size = remainder;
                
                if (stack.Size == 0) {
                    stack.Clear();
                }
            }
            
            Mesh();
        }
    }
    else if (!stack.Type || (stack.Type == HoldingStack.Type && stack.Data == HoldingStack.Data)) {
        if (HoldingSize <= DivideSlots.size()) {
            return;
        }
        
        stack.Type = HoldingStack.Type;
        stack.Data = HoldingStack.Data;
        
        DivideSlots.insert(&stack);
        
        if (DivideSlots.size() > 1) {
            int floorNum = HoldingSize / int(DivideSlots.size());
            
            for (auto &stack : DivideSlots) {
                stack->Size += floorNum;
            }
            
            HoldingStack.Size = HoldingSize % int(DivideSlots.size());
            
            Mesh();
            
            for (auto &stack : DivideSlots) {
                stack->Size -= floorNum;
            }
        }
    }
}

void Inventory::Right_Click_Stack(Stack &stack) {
    if (HoldingStack.Type) {
        if (HoldingStack.Type == stack.Type && HoldingStack.Data == stack.Data && stack.Size < MAX_STACK_SIZE) {
            stack.Size++;
        }
        else if (!stack.Type) {
            stack = Stack(HoldingStack.Type, HoldingStack.Data);
        }
        
        if (--HoldingStack.Size == 0) {
            HoldingStack.Clear();
        }
    }
    else if (stack.Type) {
        HoldingStack.Type = stack.Type;
        HoldingStack.Data = stack.Data;
        HoldingStack.Size = int(ceil(stack.Size / 2.0));
        stack.Size -= HoldingStack.Size;
        
        if (stack.Size == 0) {
            stack.Clear();
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
        if (stack.Type && --stack.Size == 0) {
            stack.Clear();
        }
    }
}

void Inventory::Switch_Slot() {
    glm::vec2 pos = barDims.xy() + glm::vec2(ActiveToolbarSlot * slotWidth.x / 2.0f, 0);
    interface.Set_Document("toolbar");
    interface.Get_Background("selectToolbar")->Move(pos, true);
    interface.Set_Document("");
}

void Inventory::Click_Handler(double x, double y, int button, int action) {
    if (HoveringSlot == -1) {
        return;
    }
    
    MouseDown = ((button == GLFW_MOUSE_BUTTON_LEFT) ? MOUSE_LEFT : MOUSE_RIGHT) * (action == GLFW_PRESS);
    
    if (HoveringSlot >= 0 && MouseDown) {
        MouseState = (HoldingStack.Type > 0);
        
        if (MouseDown == MOUSE_RIGHT || HoveringSlot == OUTPUT_SLOT) {
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
                stack->Data = HoldingStack.Data;
                stack->Size += floorNum;
            }
            
            HoldingStack.Size = remainder;
            
            if (remainder == 0) {
                HoldingStack.Clear();
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
    
    float mouseY = float(SCREEN_HEIGHT - y);
    
    HoveringSlot = -1;
    
    if (In_Range(mouseY, invDims.yw())) {
        if (In_Range(x, invDims.xz())) {
            glm::vec2 slot = glm::floor((glm::vec2(x, mouseY) - invDims.xy()) / slotWidth);
            glm::vec2 pos = slot * slotWidth + invDims.xy();
            
            HoveringSlot = int(slot.y * SLOTS_X + slot.x + SLOTS_X);
            interface.Get_Background("invHover")->Move(pos, true);
        }
        
        else if (In_Range(x, craftDims.xz())) {
            if (In_Range(mouseY, craftDims.yw())) {
                glm::vec2 slot = glm::floor((glm::vec2(x, mouseY) - craftDims.xy()) / slotWidth);
                glm::vec2 pos = slot * slotWidth + craftDims.xy();
                
                HoveringSlot = INV_SIZE + int(slot.y * 3 + slot.x);
                interface.Get_Background("invHover")->Move(pos, true);
            }
            
            else if (In_Range(mouseY, outputDims.yw())) {
                if (In_Range(x, outputDims.xz())) {
                    HoveringSlot = OUTPUT_SLOT;
                    interface.Get_Background("invHover")->Move(outputDims.xy(), true);
                }
            }
        }
    }
    else if (In_Range(mouseY, invBarDims.yw())) {
        if (In_Range(x, invBarDims.xz())) {
            glm::vec2 slot = glm::floor((glm::vec2(x, mouseY) - invBarDims.xy()) / slotWidth);
            glm::vec2 pos = slot * slotWidth + invBarDims.xy();
            
            HoveringSlot = slot.x;
            interface.Get_Background("invHover")->Move(pos, true);
        }
    }
    
    interface.Get_Background("invHover")->Opacity = 0.5f * (HoveringSlot != -1);
    
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
    
    TextElement* mouseStack = interface.Get_Text_Element("mouseStack");
    mouseStack->Opacity = float(HoldingStack.Size > 0);
    
    if (HoldingStack.Type) {
        mouseStack->Text = std::to_string(HoldingStack.Size);
        mouseStack->X = floor(x);
        mouseStack->Y = floor(mouseY);
    }
    
    interface.Get_3D_Element("mouseStack")->Mesh(HoldingStack.Type, HoldingStack.Data, x, mouseY);
    interface.Set_Document("");
}

void Inventory::Mesh() {
    int index = 0;
    
    interface.Set_Document(Is_Open ? "inventory" : "toolbar");
    
    for (auto const &stack : Inv) {
        glm::vec2 pos;
        
        if (Is_Open) {
            if (index < SLOTS_X) {
                pos = invBarDims.xy() + glm::vec2(index % SLOTS_X, 0) * slotWidth + slotPad;
            }
            else {
                pos = invDims.xy() + glm::vec2(index % SLOTS_X, index / SLOTS_X - 1) * slotWidth + slotPad;
            }
        }
        else {
            pos = barDims.xy() + glm::vec2(index % SLOTS_X, 0) * slotWidth / 2.0f + glm::vec2(slotPad.x, 0) / 2.0f;
        }
        
        std::string textName = std::to_string(index);
        interface.Get_Text_Element(textName)->Opacity = float(stack.Type > 0);
        interface.Get_3D_Element(textName)->Mesh(stack.Type, stack.Data, pos + glm::vec2(0, 10));
        
        if (stack.Type) {
            interface.Get_Text_Element(textName)->Text = std::to_string(stack.Size);
        }
        
        if (!Is_Open && index == 9) {
            break;
        }
        
        index++;
    }
    
    if (Is_Open) {
        index = 0;
        
        for (auto const &stack : Craft) {
            std::string textName = std::to_string(INV_SIZE + index);
            glm::vec2 pos = craftDims.xy() + glm::vec2(index % 3, index / 3) * slotWidth + slotPad;
            
            interface.Get_Text_Element(textName)->Opacity = float(stack.Type > 0);
            interface.Get_3D_Element(textName)->Mesh(stack.Type, stack.Data, pos + glm::vec2(0, 10));
            
            if (stack.Type) {
                interface.Get_Text_Element(textName)->Text = std::to_string(stack.Size);
            }
            
            ++index;
        }
        
        std::string outputName = std::to_string(OUTPUT_SLOT);
        interface.Get_Text_Element(outputName)->Opacity = float(CraftingOutput.Type > 0);
        interface.Get_3D_Element(outputName)->Mesh(CraftingOutput.Type, CraftingOutput.Data, outputDims.xy() + glm::vec2(invPad.x, invPad.y * 2));
        
        if (CraftingOutput.Type) {
            interface.Get_Text_Element(outputName)->Text = std::to_string(CraftingOutput.Size);
        }
    }
    
    interface.Set_Document("");
}

void Inventory::Draw() {
    interface.Draw_Document(Is_Open ? "inventory" : "toolbar");
}