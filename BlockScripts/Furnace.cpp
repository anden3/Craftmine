#include "Furnace.h"

#include "UI.h"
#include "Chat.h"
#include "main.h"
#include "Chunk.h"
#include "Blocks.h"
#include "Player.h"
#include "Interface.h"
#include "Comparators.h"

static const float SMELT_TIME = 10.0f;

struct FurnaceInstance {
    float Progress = 0.0f;
    float BurnTimeRemaining = 0.0f;

    Stack Fuel   = Stack();
    Stack Input  = Stack();
    Stack Output = Stack();
};

static std::map<glm::ivec3, FurnaceInstance, VectorComparator> FurnaceData;

static Slot* Fuel;
static Slot* Input;
static Slot* Output;

static FurnaceInstance* CurrentFurnace = nullptr;

void Update_Contents() {
    CurrentFurnace->Fuel   = Fuel->Contents;
    CurrentFurnace->Input  = Input->Contents;
    CurrentFurnace->Output = Output->Contents;
}

void Furnace::Init() {
    Data arrow = {
        0, 0, 5, 0, 5, 20,
        0, 0, 5, 20, 0, 20,
        -5, 0, 3, -8, 10, 0
    };

    glm::vec2 bgPad = Scale(10);

    glm::vec4 bgDims = glm::vec4(Scale(200, 400), Scale(180, 180));

    Interface::Set_Document("furnace");
        Interface::Add_Background("bg", glm::vec4(bgDims.xy(), bgDims.zw()), true, {0, 0}, bgPad);

        Interface::Add_Slot("fuel", Scale(240, 475), Scale_Y(40));
        Interface::Add_Slot("input", Scale(300, 520), Scale_Y(40));
        Interface::Add_Slot("output", Scale(300, 430), Scale_Y(40));

        Fuel   = Interface::Get_Slot("fuel");
        Input  = Interface::Get_Slot("input");
        Output = Interface::Get_Slot("output");

        Output->OutputOnly = true;

        Interface::Add_Custom("arrow", Scale_X(320), Scale_Y(495), arrow);
        Interface::Get_Custom("arrow")->Color = glm::vec3(1);
    Interface::Set_Document("");
}

void Furnace::Update() {
    if (CurrentFurnace != nullptr) {
        Update_Contents();
    }

    for (auto &furnace : FurnaceData) {
        FurnaceInstance &f = furnace.second;

        if (f.BurnTimeRemaining > 0.0f) {
            f.BurnTimeRemaining -= static_cast<float>(DeltaTime);
        }

        if (f.BurnTimeRemaining < 0.0f) {
            f.BurnTimeRemaining = 0.0f;
        }

        const Block* inputType = Blocks::Get_Block(f.Input);

        if (!f.Input.Type || !inputType->Smeltable) {
            continue;
        }

        if (f.Output.Type && f.Output != inputType->SmeltResult) {
            return;
        }

        if (f.BurnTimeRemaining > 0.0f) {
            f.Progress += static_cast<float>(DeltaTime);

            if (f.Progress >= SMELT_TIME) {
                f.Progress = 0.0f;

                if (f.Output.Type) {
                    f.Output.Size += 1;
                }
                else {
                    f.Output = inputType->SmeltResult;
                }

                f.Input.Decrease();
                Input->Set_Contents(f.Input);
                Output->Set_Contents(f.Output);
            }
        }

        const Block* fuelType = Blocks::Get_Block(f.Fuel);

        if (fuelType->BurnTime == 0.0f) {
            continue;
        }

        if (f.BurnTimeRemaining == 0.0f && f.Fuel.Type) {
            f.Fuel.Decrease();
            Fuel->Set_Contents(f.Fuel);

            f.BurnTimeRemaining = fuelType->BurnTime;
        }
    }
}

void Furnace::Right_Click() {
    UI::CustomDocument = "furnace";
    UI::Toggle_Mouse(true);

    CurrentFurnace = &FurnaceData[Get_World_Pos(player.LookingChunk, player.LookingTile)];

    Fuel->Set_Contents(CurrentFurnace->Fuel);
    Input->Set_Contents(CurrentFurnace->Input);
    Output->Set_Contents(CurrentFurnace->Output);
}

void Furnace::Close() {
    Update_Contents();
    CurrentFurnace = nullptr;

    UI::CustomDocument = "";
    UI::Toggle_Mouse(false);
}
