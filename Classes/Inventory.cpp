#include "Inventory.h"

#include <glm/gtc/matrix_transform.hpp>

#include <regex>
#include <set>

#include <GLFW/glfw3.h>

#include "main.h"
#include "Blocks.h"
#include "Interface.h"

const int SLOTS_X = 10;
const int SLOTS_Y = 7;

const int INV_SIZE = SLOTS_X * SLOTS_Y;
const int MAX_STACK_SIZE = 64;

static float BLOCK_SCALE;

static glm::vec4 barDims;
static glm::vec4 invDims;
static glm::vec4 craftDims;
static glm::vec4 outputDims;
static glm::vec4 invBarDims;
static glm::vec2 slotWidth;
static glm::vec2 invPad;
static glm::vec2 slotPad;
static glm::vec2 textPad;

const int OUTPUT_SLOT = INV_SIZE + 9;

enum MouseButtons {
    NONE,
    MOUSE_LEFT,
    MOUSE_RIGHT
};

static int MouseDown = NONE;

static bool MouseState = 0;

static int LastSlot = -1;
static int StartSlot = -1;

static std::set<Stack*> DivideSlots;
static int HoldingSize = 0;

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
                if (c != ' ') {
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

static std::map<int, std::vector<Recipe>> Recipes = {
    // %   == Any number (including zero) of zeroes.
    // ()  == Capture expression
    // ()x == Repeating captured expression x times.
    // []  == Repeating n times, where n is any of the numbers in the set.

    {1, std::vector<Recipe> {
        {"% 17 %", {5, 4}}, // Wooden Planks
    }},

    {2, std::vector<Recipe> {
        {"% 5 (0)2 5 %",     {280, 4}}, // Stick
        {"% 280 (0)2 4 %",   {69}},     // Lever
        {"% 263 (0)2 280 %", {50, 4}},  // Torch
    }},

    {3, std::vector<Recipe> {
        {"% 265 (0 0 280)2 %", {256}},   // Iron Shovel
        {"(0)[0,3,6] (1)3 %",  {44, 6}}, // Stone Slab
    }},

    {4, std::vector<Recipe> {
        {"% (5)2 0 (5)2 %",   {58}}, // Crafting Table
        {"% (12)2 0 (12)2 %", {24}}, // Sandstone
    }},

    {5, std::vector<Recipe> {
        {"% (265)2 0 265 280 (0)2 280 %", {258}}, // Iron Axe
        {"(265)3 (0 280 0)2",             {257}}, // Iron Pickaxe
    }},

    {6, std::vector<Recipe> {
        {"(0)[0,3] (35)3 (5)3 %", {26}}, // Bed
    }},

    {7, std::vector<Recipe> {
        {"265 0 (265)2 280 (265)2 0 265", {66, 16}}, // Rail
        {"280 0 (280)5 0 280",            {65, 4}},  // Ladder
    }},

    {8, std::vector<Recipe> {
        {"(4)4 0 (4)4", {61}}, // Furnace
        {"(5)4 0 (5)4", {54}}, // Chest
    }},

    {9, std::vector<Recipe> {
        {"(264)9", {57}}, // Diamond Block
        {"(265)9", {42}}, // Iron Block
        {"(266)9", {41}}, // Gold Block
    }}
};

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

    Interface::Set_Document("toolbar");

    for (int i = 0; i < SLOTS_X; i++) {
        std::string name = std::to_string(i);
        glm::vec2 pos(std::floor(barDims.x + i * (slotWidth.x / 2.0f)), std::floor(barDims.y));

        Interface::Add_Text(name, "0", pos + textPad / 2.0f);
        Interface::Get_Text_Element(name)->Opacity = 0.0f;
        Interface::Add_3D_Element(name, 0, 0, pos + glm::vec2(0, slotPad.y), BLOCK_SCALE / 2.0f);
    }

    Interface::Set_Document("inventory");

    for (int i = 0; i < INV_SIZE; i++) {
        std::string name = std::to_string(i);
        glm::vec2 pos(
            std::floor(invDims.x + (i % SLOTS_X) * slotWidth.x),
            std::floor((i < SLOTS_X) ? invBarDims.y : invDims.y + ((i / SLOTS_X) - 1) * slotWidth.y)
        );

        Inv.push_back(Stack());
        Interface::Add_Text(name, "0", pos + slotPad);
        Interface::Get_Text_Element(name)->Opacity = 0.0f;
        Interface::Add_3D_Element(name, 0, 0, pos + glm::vec2(0, slotPad.y), BLOCK_SCALE);
    }

    for (int i = 0; i < 9; i++) {
        std::string name = std::to_string(INV_SIZE + i);
        glm::vec2 pos = glm::vec2(i % 3, i / 3) * slotWidth + craftDims.xy() + slotPad;

        Craft.push_back(Stack());
        Interface::Add_Text(name, "0", pos);
        Interface::Get_Text_Element(name)->Opacity = 0.0f;
        Interface::Add_3D_Element(name, 0, 0, pos, BLOCK_SCALE);
    }

    Interface::Add_Text(std::to_string(INV_SIZE + 9), "0", outputDims.xy() + slotPad);
    Interface::Get_Text_Element(std::to_string(INV_SIZE + 9))->Opacity = 0.0f;
    Interface::Add_3D_Element(std::to_string(INV_SIZE + 9), 0, 0, outputDims.xy() + glm::vec2(slotPad.x, slotPad.y * 2), BLOCK_SCALE);

    Interface::Add_Background("invInv", glm::vec4(invDims.xy() - invPad, invDims.zw() + invPad * 2.0f), true, slotWidth, invPad);
    Interface::Add_Background("invCraft", glm::vec4(craftDims.xy() - invPad, craftDims.zw() + invPad * 2.0f), true, slotWidth, invPad);
    Interface::Add_Background("invOutput", glm::vec4(outputDims.xy() - invPad, outputDims.zw() + invPad * 2.0f), true, slotWidth, invPad);
    Interface::Add_Background("invToolbar", glm::vec4(invBarDims.xy() - invPad, invBarDims.zw() + invPad * 2.0f), true, slotWidth, invPad);

    Interface::Add_Background("invHover", glm::vec4(0, 0, slotWidth));
    Interface::Get_Background("invHover")->Color = glm::vec3(0.7f);

    Interface::Add_Text("mouseStack", std::to_string(HoldingStack.Size), 0, 0);
    Interface::Get_Text_Element("mouseStack")->Opacity = 0.0f;

    Interface::Add_3D_Element("mouseStack", 0, 0, 0, 0, BLOCK_SCALE);

    Interface::Set_Document("toolbar");
        Interface::Add_Background("bgToolbar", barDims, true, slotWidth / 2.0f);
        Interface::Add_Background("selectToolbar", glm::vec4(barDims.xy(), slotWidth.x / 2.0f, barDims.w), true, slotWidth / 2.0f);
        Interface::Get_Background("selectToolbar")->Color = glm::vec3(0.7f);
    Interface::Set_Document("");

    Switch_Slot();
}

void Inventory::Clear() {
    for (unsigned long i = 0; i < Inv.size(); ++i) {
        Inv[i].Clear();
    }

    Mesh();
}

void Inventory::Add_Stack(int type, int typeData, int size) {
    unsigned long index = 0;

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

        ++index;
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

        ++index;
    }
}

void Inventory::Decrease_Size(int slot) {
    if (slot == -1) {
        slot = ActiveToolbarSlot;
    }

    unsigned long uSlot = static_cast<unsigned long>(slot);

    --Inv[uSlot].Size;

    if (Inv[uSlot].Size == 0) {
        Inv[uSlot].Clear();
    }

    Mesh();
}

Stack Inventory::Get_Info(int slot) {
    if (slot == -1) {
        slot = ActiveToolbarSlot;
    }

    return Inv[static_cast<unsigned long>(slot)];
}

Stack& Inventory::Get_Stack(int slot) {
    unsigned long uSlot = static_cast<unsigned long>(slot);
    return (slot >= INV_SIZE) ? Craft[uSlot - INV_SIZE] : Inv[uSlot];
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
    unsigned long uSlot = static_cast<unsigned long>(slot);
    Stack &stack = (slot >= INV_SIZE) ? Craft[uSlot - INV_SIZE] : Inv[uSlot];

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
        if (HoldingSize <= static_cast<int>(DivideSlots.size())) {
            return;
        }

        stack.Type = HoldingStack.Type;
        stack.Data = HoldingStack.Data;

        DivideSlots.insert(&stack);

        if (DivideSlots.size() > 1) {
            int floorNum = HoldingSize / int(DivideSlots.size());

            for (auto &divStack : DivideSlots) {
                divStack->Size += floorNum;
            }

            HoldingStack.Size = HoldingSize % int(DivideSlots.size());

            Mesh();

            for (auto &divStack : DivideSlots) {
                divStack->Size -= floorNum;
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
            stack = Stack(HoldingStack.Type, HoldingStack.Data, 1);
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
            // Checks if recipe result exists :P
            if (Blocks::Exists(recipe.Result.Type, recipe.Result.Data)) {
                CraftingOutput = recipe.Result;
                Mesh();
            }

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
    Interface::Set_Document("toolbar");
    Interface::Get_Background("selectToolbar")->Move(pos, true);
    Interface::Set_Document("");
}

void Inventory::Click_Handler(int button, int action) {
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

    Mouse_Handler(-1, -1);
}

void Inventory::Mouse_Handler(double x, double y) {
    Interface::Set_Document("inventory");

    if (x == -1 || y == -1) {
        x = MousePos.x;
        y = MousePos.y;
    }

    MousePos = glm::dvec2(x, y);

    double mouseY = SCREEN_HEIGHT - y;

    HoveringSlot = -1;

    if (In_Range(mouseY, invDims.yw())) {
        if (In_Range(x, invDims.xz())) {
            glm::vec2 slot = glm::floor((glm::vec2(x, mouseY) - invDims.xy()) / slotWidth);
            glm::vec2 pos = slot * slotWidth + invDims.xy();

            HoveringSlot = int(slot.y * SLOTS_X + slot.x + SLOTS_X);
            Interface::Get_Background("invHover")->Move(pos, true);
        }

        else if (In_Range(x, craftDims.xz())) {
            if (In_Range(mouseY, craftDims.yw())) {
                glm::vec2 slot = glm::floor((glm::vec2(x, mouseY) - craftDims.xy()) / slotWidth);
                glm::vec2 pos = slot * slotWidth + craftDims.xy();

                HoveringSlot = INV_SIZE + int(slot.y * 3 + slot.x);
                Interface::Get_Background("invHover")->Move(pos, true);
            }

            else if (In_Range(mouseY, outputDims.yw())) {
                if (In_Range(x, outputDims.xz())) {
                    HoveringSlot = OUTPUT_SLOT;
                    Interface::Get_Background("invHover")->Move(outputDims.xy(), true);
                }
            }
        }
    }
    else if (In_Range(mouseY, invBarDims.yw())) {
        if (In_Range(x, invBarDims.xz())) {
            glm::ivec2 slot = glm::floor((glm::vec2(x, mouseY) - invBarDims.xy()) / slotWidth);
            glm::vec2 pos = static_cast<glm::vec2>(slot) * slotWidth + invBarDims.xy();

            HoveringSlot = slot.x;
            Interface::Get_Background("invHover")->Move(pos, true);
        }
    }

    Interface::Get_Background("invHover")->Opacity = 0.5f * (HoveringSlot != -1);

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

    TextElement* mouseStack = Interface::Get_Text_Element("mouseStack");
    mouseStack->Opacity = float(HoldingStack.Size > 0);

    if (HoldingStack.Type) {
        mouseStack->X = static_cast<float>(std::floor(x));
        mouseStack->Y = static_cast<float>(std::floor(mouseY));
        mouseStack->Set_Text(std::to_string(HoldingStack.Size));
    }

    Interface::Get_3D_Element("mouseStack")->Mesh(
        HoldingStack.Type, HoldingStack.Data, static_cast<float>(x), static_cast<float>(mouseY)
    );
    Interface::Set_Document("");
}

void Inventory::Load(const JSONValue &data, std::vector<Stack> &storage) {
    for (auto it = data.begin(); it != data.end(); ++it) {
        if (it.value().size() == 3) {
            storage[std::stoul(it.key())] = {
                it.value()[0], it.value()[1], it.value()[2]
            };
        }
        else {
            storage[std::stoul(it.key())] = {
                it.value()[0].get<int>(), it.value()[1]
            };
        }
    }

    Mesh();
}

void Inventory::Mesh() {
    int index = 0;

    Interface::Set_Document(Is_Open ? "inventory" : "toolbar");

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
        Interface::Get_Text_Element(textName)->Opacity = static_cast<float>(stack.Type > 0);
        Interface::Get_3D_Element(textName)->Mesh(stack.Type, stack.Data, pos + glm::vec2(0, 10));

        if (stack.Type) {
            TextElement* text = Interface::Get_Text_Element(textName);

            if (text->Text != std::to_string(stack.Size)) {
                text->Set_Text(std::to_string(stack.Size));
            }
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

            Interface::Get_Text_Element(textName)->Opacity = static_cast<float>(stack.Type > 0);
            Interface::Get_3D_Element(textName)->Mesh(
                stack.Type, stack.Data, pos + glm::vec2(0, 10)
            );

            if (stack.Type) {
                TextElement* text = Interface::Get_Text_Element(textName);

                if (text->Text != std::to_string(stack.Size)) {
                    text->Set_Text(std::to_string(stack.Size));
                }
            }

            ++index;
        }

        std::string outputName = std::to_string(OUTPUT_SLOT);
        Interface::Get_Text_Element(outputName)->Opacity = float(CraftingOutput.Type > 0);
        Interface::Get_3D_Element(outputName)->Mesh(
            CraftingOutput.Type, CraftingOutput.Data,
            outputDims.xy() + glm::vec2(invPad.x, invPad.y * 2)
        );

        if (CraftingOutput.Type) {
            TextElement* text = Interface::Get_Text_Element(outputName);

            if (text->Text != std::to_string(CraftingOutput.Size)) {
                text->Set_Text(std::to_string(CraftingOutput.Size));
            }
        }
    }

    Interface::Set_Document("");
}

void Inventory::Draw() {
    Interface::Draw_Document(Is_Open ? "inventory" : "toolbar");
}
