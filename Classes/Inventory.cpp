#include "Inventory.h"

#include <set>

#include <GLFW/glfw3.h>

#include "main.h"
#include "Blocks.h"
#include "Interface.h"

const int SLOTS_X = 10;
const int SLOTS_Y = 7;

const int INV_SIZE = SLOTS_X * SLOTS_Y;
const int MAX_STACK_SIZE = 64;

bool Inventory::Is_Open = false;
int Inventory::ActiveToolbarSlot = 0;

std::vector<Slot*> Inv;
std::vector<Slot*> Craft;
std::vector<Slot*> Toolbar;

Stack Inventory::HoldingStack;
Slot* Inventory::CraftingOutput;

static Slot* CurrentSlot = nullptr;

static int MouseButton = 0;
static bool MouseDown = false;
static bool MovedSlot = false;

static std::set<Slot*> DivideSlots;
static int HoldingSize = 0;

void Click_Slot(int slot, int button);
void Check_Crafting();
void Craft_Item();

Slot* Get_Slot(int slot);

void Swap_Stacks(Slot* slot);
bool Left_Click_Stack(Stack* a, Stack* b);
void Right_Click_Stack(Slot* slot);

void Left_Drag(Slot* slot);

void Inventory::Init() {
    float blockScale = Scale_X(40);

    glm::vec2 textPad   = Scale(2.5f);
    glm::vec2 slotPad   = Scale(5);
    glm::vec2 slotWidth = Scale(40);

    glm::vec4 barDims    = glm::vec4(Scale(520, 40),   Scale(400, 40));
    glm::vec4 invDims    = glm::vec4(Scale(520, 340),  Scale(400, 240));
    glm::vec4 craftDims  = glm::vec4(Scale(1000, 460), Scale(120));
    glm::vec4 outputDims = glm::vec4(Scale(1040, 340), Scale(40));
    glm::vec4 invBarDims = glm::vec4(Scale(520, 260),  Scale(400, 40));

    Interface::Set_Document("toolbar");

    for (int i = 0; i < SLOTS_X; i++) {
        std::string name = std::to_string(i);
        glm::vec2 pos(std::floor(barDims.x + i * slotWidth.x), std::floor(barDims.y));

        Interface::Add_Slot(name, pos + textPad, 40);
        Toolbar.push_back(Interface::Get_Slot(name));
    }

    Interface::Set_Document("inventory");

    for (int i = 0; i < INV_SIZE; i++) {
        std::string name = std::to_string(i);
        glm::vec2 pos(
            std::floor(invDims.x + (i % SLOTS_X) * slotWidth.x),
            std::floor((i < SLOTS_X) ? invBarDims.y : invDims.y + ((i / SLOTS_X) - 1) * slotWidth.y)
        );

        Interface::Add_Slot(name, pos + slotPad, 40);

        if (i < SLOTS_X) {
            Interface::Get_Slot(name)->SyncedSlot = Toolbar[i];
        }

        Inv.push_back(Interface::Get_Slot(name));
    }

    for (int i = 0; i < 9; i++) {
        std::string name = std::to_string(INV_SIZE + i);
        glm::vec2 pos = glm::vec2(i % 3, i / 3) * slotWidth + craftDims.xy() + slotPad;

        Interface::Add_Slot(name, pos, 40);
        Interface::Get_Slot(name)->CraftingInput = true;
        Craft.push_back(Interface::Get_Slot(name));
    }

    Interface::Add_Slot(std::to_string(INV_SIZE + 9), outputDims.xy() + slotPad, 40);
    CraftingOutput = Interface::Get_Slot(std::to_string(INV_SIZE + 9));
    CraftingOutput->CraftingOutput = true;
    CraftingOutput->OutputOnly = true;

    Interface::Add_Text("mouseStack", std::to_string(HoldingStack.Size), 0, 0);
    Interface::Get_Text_Element("mouseStack")->Opacity = 0.0f;
    Interface::Add_3D_Element("mouseStack", 0, 0, 0, 0, blockScale);

    Switch_Slot(0);
}

void Inventory::Clear() {
    for (unsigned long i = 0; i < Inv.size(); ++i) {
        Inv[i]->Contents.Clear();
        Inv[i]->Mesh();
    }
}

void Inventory::Add_Stack(int type, int typeData, int size) {
    for (auto &slot : Inv) {
        if (slot->Contents.Type == type && slot->Contents.Data == typeData) {
            if (slot->Contents.Size < MAX_STACK_SIZE) {
                if (slot->Contents.Size + size <= MAX_STACK_SIZE) {
                    slot->Contents.Size += size;
                    slot->Mesh();
                    return;
                }
                else {
                    size -= MAX_STACK_SIZE - slot->Contents.Size;
                    slot->Contents.Size = MAX_STACK_SIZE;
                    slot->Mesh();
                }
            }
        }
    }

    for (auto const &slot : Inv) {
        if (slot->Contents.Type == 0) {
            slot->Contents.Type = type;
            slot->Contents.Data = typeData;

            if (size <= MAX_STACK_SIZE) {
                slot->Contents.Size = size;
                slot->Mesh();
                return;
            }
            else {
                slot->Contents.Size = MAX_STACK_SIZE;
                size -= MAX_STACK_SIZE;
                slot->Mesh();
            }
        }
    }
}

void Inventory::Decrease_Size(int slot) {
    if (slot == -1) {
        slot = ActiveToolbarSlot;
    }

    unsigned long uSlot = static_cast<unsigned long>(slot);

    Inv[uSlot]->Contents.Size -= 1;

    if (Inv[uSlot]->Contents.Size == 0) {
        Inv[uSlot]->Contents.Clear();
    }

    Inv[uSlot]->Mesh();
}

Stack* Inventory::Get_Info(int slot) {
    if (slot == -1) {
        slot = ActiveToolbarSlot;
    }

    return &Inv[static_cast<unsigned long>(slot)]->Contents;
}

Slot* Get_Slot(int slot) {
    unsigned long uSlot = static_cast<unsigned long>(slot);
    return (slot >= INV_SIZE) ? Craft[uSlot - INV_SIZE] : Inv[uSlot];
}

void Inventory::Click_Slot(Slot* slot) {
    if (slot->OutputOnly) {
        if (slot->Contents.Type == 0) {
            return;
        }

        bool craft = false;

        if (keys[GLFW_KEY_LEFT_SHIFT]) {
            Add_Stack(&slot->Contents);
            slot->Contents.Clear();

            if (slot->CraftingOutput) {
                craft = true;
            }
        }

        else if (!HoldingStack.Type) {
            Swap_Stacks(slot);

            if (slot->CraftingOutput) {
                craft = true;
            }
        }

        else if (Left_Click_Stack(&slot->Contents, &HoldingStack)) {
            if (slot->CraftingOutput) {
                craft = true;
            }
        }

        slot->Mesh();

        if (craft) {
            Craft_Item();
            Check_Crafting();
        }
    }
    else {
        if (MouseButton == GLFW_MOUSE_BUTTON_LEFT) {
            if (Left_Click_Stack(&HoldingStack, &slot->Contents)) {
                slot->Mesh();
            }
            else {
                Swap_Stacks(slot);
            }
        }
        else if (MouseButton == GLFW_MOUSE_BUTTON_RIGHT) {
            Right_Click_Stack(slot);
        }
    }

    if (slot->CraftingInput) {
        Check_Crafting();
    }
}

void Inventory::Press_Slot(Slot* slot, int button) {
    CurrentSlot = slot;
    MouseDown = true;
    MouseButton = button;
    MovedSlot = false;

    if (MouseButton == GLFW_MOUSE_BUTTON_LEFT) {
        DivideSlots.insert(slot);
        HoldingSize = HoldingStack.Size;
    }
    else {
        Click_Slot(slot);
    }
}

void Inventory::Dragging_Slot(Slot* slot) {
    if (slot != CurrentSlot) {
        MovedSlot = true;

        if (MouseButton == GLFW_MOUSE_BUTTON_LEFT) {
            Left_Drag(CurrentSlot);
            Left_Drag(slot);
        }
        else {
            Click_Slot(slot);
        }

        CurrentSlot = slot;
    }
}

void Inventory::Release_Slot() {
    if (!MouseDown) {
        return;
    }

    MouseDown = false;

    if (MouseButton == GLFW_MOUSE_BUTTON_LEFT) {
        if (MovedSlot) {
            Left_Drag(CurrentSlot);

            if (DivideSlots.size() >= 2) {
                int floorNum = HoldingSize / int(DivideSlots.size());
                int remainder = HoldingSize % int(DivideSlots.size());

                for (auto &slot : DivideSlots) {
                    Stack* stack = &slot->Contents;

                    stack->Type = HoldingStack.Type;
                    stack->Data = HoldingStack.Data;
                    stack->Size += floorNum;
                    slot->Mesh();
                }

                HoldingStack.Size = remainder;

                if (remainder == 0) {
                    HoldingStack.Clear();
                }

                Check_Crafting();
            }
        }
        else {
            Click_Slot(CurrentSlot);
        }
    }

    DivideSlots.clear();
    MovedSlot = false;
    CurrentSlot = nullptr;
}

void Swap_Stacks(Slot* slot) {
    Stack toBePlaced = Inventory::HoldingStack;
    Inventory::HoldingStack = slot->Contents;
    slot->Set_Contents(toBePlaced);
}

bool Left_Click_Stack(Stack* a, Stack* b) {
    if (a->Type != 0 && a->Type == b->Type && a->Data == b->Data) {
        if (b->Size < MAX_STACK_SIZE) {
            if (a->Size <= MAX_STACK_SIZE - b->Size) {
                b->Size += a->Size;
                a->Clear();
            }
            else {
                int remainder = a->Size - (MAX_STACK_SIZE - b->Size);
                b->Size = MAX_STACK_SIZE;
                a->Size = remainder;
            }

            return true;
        }
    }

    return false;
}

void Left_Drag(Slot* slot) {
    Stack* stack = &slot->Contents;

    if (HoldingSize == 0) {
        if (Inventory::HoldingStack.Type == 0 || (stack->Type == Inventory::HoldingStack.Type && stack->Data == Inventory::HoldingStack.Data)) {
            Inventory::HoldingStack.Type = stack->Type;
            Inventory::HoldingStack.Data = stack->Data;

            if (Inventory::HoldingStack.Size <= MAX_STACK_SIZE - stack->Size) {
                Inventory::HoldingStack.Size += stack->Size;
                stack->Clear();
            }
            else {
                int remainder = stack->Size - (MAX_STACK_SIZE - Inventory::HoldingStack.Size);
                Inventory::HoldingStack.Size = MAX_STACK_SIZE;
                stack->Size = remainder;

                if (stack->Size == 0) {
                    stack->Clear();
                }
            }

            slot->Mesh();

            if (slot->CraftingInput) {
                Check_Crafting();
            }
            else if (slot->CraftingOutput) {
                Craft_Item();
                Check_Crafting();
            }
        }
    }
    else if (!stack->Type || (stack->Type == Inventory::HoldingStack.Type && stack->Data == Inventory::HoldingStack.Data)) {
        if (HoldingSize <= static_cast<int>(DivideSlots.size())) {
            return;
        }

        if (slot->OutputOnly) {
            return;
        }

        stack->Type = Inventory::HoldingStack.Type;
        stack->Data = Inventory::HoldingStack.Data;

        DivideSlots.insert(slot);

        if (DivideSlots.size() >= 1) {
            bool inCraftingArea = false;
            int floorNum = HoldingSize / int(DivideSlots.size());

            for (auto &divSlot : DivideSlots) {
                divSlot->Contents.Size += floorNum;
                divSlot->Mesh();

                if (divSlot->CraftingInput) {
                    inCraftingArea = true;
                }
            }

            if (inCraftingArea) {
                Check_Crafting();
            }

            Inventory::HoldingStack.Size = HoldingSize % int(DivideSlots.size());

            for (auto &divSlot : DivideSlots) {
                divSlot->Contents.Size -= floorNum;
            }
        }
    }
}

void Right_Click_Stack(Slot* slot) {
    Stack* stack = &slot->Contents;

    if (Inventory::HoldingStack.Type) {
        if (Inventory::HoldingStack.Type == stack->Type && Inventory::HoldingStack.Data == stack->Data && stack->Size < MAX_STACK_SIZE) {
            ++stack->Size;
        }
        else if (!stack->Type) {
            stack->Type = Inventory::HoldingStack.Type;
            stack->Data = Inventory::HoldingStack.Data;
            stack->Size = 1;
        }

        slot->Mesh();

        if (--Inventory::HoldingStack.Size == 0) {
            Inventory::HoldingStack.Clear();
        }
    }
    else if (stack->Type) {
        Inventory::HoldingStack.Type = stack->Type;
        Inventory::HoldingStack.Data = stack->Data;
        Inventory::HoldingStack.Size = int(ceil(stack->Size / 2.0));
        stack->Size -= Inventory::HoldingStack.Size;

        if (stack->Size == 0) {
            stack->Clear();
        }

        slot->Mesh();
    }
}

void Check_Crafting() {
    std::string grid = "";
    int blocks = 0;

    for (auto const &slot : Craft) {
        grid.insert(0, std::to_string(slot->Contents.Type) + ",");
        blocks += slot->Contents.Type > 0;
    }

	const Block* result = Blocks::Check_Crafting(grid);

	if (result != nullptr) {
        Inventory::CraftingOutput->Contents = Stack(
            result->ID, result->Data, result->CraftingYield
        );
        Inventory::CraftingOutput->Mesh();
	}
	else if (Inventory::CraftingOutput->Contents.Type) {
		Inventory::CraftingOutput->Contents.Clear();
        Inventory::CraftingOutput->Mesh();
	}
}

void Craft_Item() {
    int index = 0;

    for (auto &slot : Craft) {
        if (slot->Contents.Type) {
            slot->Contents.Size -= 1;

            if (slot->Contents.Size == 0) {
                slot->Contents.Clear();
            }

            slot->Mesh();
        }
    }
}

void Inventory::Switch_Slot(int slot) {
    Interface::Set_Document("toolbar");

    Interface::Get_Slot(std::to_string(ActiveToolbarSlot))->Stop_Hover();
    ActiveToolbarSlot = slot;
    Interface::Get_Slot(std::to_string(ActiveToolbarSlot))->Hover();

    Interface::Set_Document("");
}

void Inventory::Mouse_Handler(double x, double y) {
    static bool meshModel = true;

    Interface::Set_Document("inventory");
    TextElement* mouseStack = Interface::Get_Text_Element("mouseStack");
    OrthoElement* mouseModel = Interface::Get_3D_Element("mouseStack");
    Interface::Set_Document("");

    mouseStack->Opacity = float(Inventory::HoldingStack.Size > 0);

    if (!HoldingStack.Type) {
        if (meshModel) {
            mouseModel->Mesh(
                HoldingStack.Type, HoldingStack.Data,
                static_cast<float>(x), static_cast<float>(SCREEN_HEIGHT - y)
            );

            meshModel = false;
        }

        return;
    }

    meshModel = true;

    mouseStack->X = static_cast<float>(std::floor(x));
    mouseStack->Y = static_cast<float>(std::floor(SCREEN_HEIGHT - y));
    mouseStack->Set_Text(std::to_string(Inventory::HoldingStack.Size));

    mouseModel->Mesh(
        HoldingStack.Type, HoldingStack.Data, static_cast<float>(x), static_cast<float>(SCREEN_HEIGHT - y)
    );
}

void Inventory::Save(nlohmann::json &dest, std::string type) {
    std::vector<Slot*> &storage = Inv;
    int currentSlot = 0;

    if (type == "Crafting") {
        storage = Craft;
        currentSlot = INV_SIZE;
    }

    for (auto const &slot : storage) {
        if (!slot->Contents.Type) {
            ++currentSlot;
            continue;
        }

        if (slot->Contents.Data) {
            dest["Storage"][type][std::to_string(currentSlot)] = {
                slot->Contents.Type, slot->Contents.Data, slot->Contents.Size
            };
        }
        else {
            dest["Storage"][type][std::to_string(currentSlot)] = {
                slot->Contents.Type, slot->Contents.Size
            };
        }

        slot->Mesh();
        ++currentSlot;
    }
}

void Inventory::Load(const JSONValue &data) {
    Interface::Set_Document("inventory");

    for (auto it = data.begin(); it != data.end(); ++it) {
        if (it.value()[0].get<int>() > 0) {
            Slot* slot = Interface::Get_Slot(it.key());

            if (it.value().size() == 3) {
                slot->Set_Contents(Stack(it.value()[0], it.value()[1], it.value()[2]));
            }
            else {
                slot->Set_Contents(Stack(it.value()[0], 0, it.value()[1]));
            }
        }
    }

    Interface::Set_Document("");
}

void Inventory::Draw() {
    Interface::Draw_Document(Is_Open ? "inventory" : "toolbar");
}
