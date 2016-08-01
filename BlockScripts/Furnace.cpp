#include "Furnace.h"

#include "UI.h"
#include "Chat.h"
#include "main.h"
#include "Interface.h"

void Furnace::Init() {
    glm::vec2 bgPad = Scale(10);
    glm::vec2 slotWidth = Scale(80);

    glm::vec4 bgDims = glm::vec4(Scale(520, 250), Scale(400, 400));

    Interface::Set_Document("furnace");

    Interface::Add_Background("furnaceBg", glm::vec4(bgDims.xy() - bgPad, bgDims.zw() + bgPad * 2.0f), true, {0, 0}, bgPad);

    Interface::Add_Slot("slot1", Scale(630, 500), Scale_Y(40), 1, 0, 1);
    Interface::Add_Slot("slot2", Scale(730, 500), Scale_Y(40), 2, 0, 2);
    Interface::Add_Slot("slot3", Scale(630, 300), Scale_Y(40), 3, 0, 3);
    Interface::Add_Slot("slot4", Scale(730, 300), Scale_Y(40), 4, 0, 4);

    Interface::Set_Document("");
}

void Furnace::Right_Click() {
    UI::CustomDocument = "furnace";
    UI::Toggle_Mouse(true);
}