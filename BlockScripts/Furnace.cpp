#include "Furnace.h"

#include "UI.h"
#include "Chat.h"
#include "main.h"
#include "Interface.h"

void Furnace::Init() {
    glm::vec2 bgPad = Scale(10);
    glm::vec2 slotWidth = Scale(80);

    glm::vec4 bgDims = glm::vec4(Scale(320), Scale(800, 480));

    Interface::Set_Document("furnace");

    Interface::Add_Background("furnaceBg", glm::vec4(bgDims.xy() - bgPad, bgDims.zw() + bgPad * 2.0f), true, {0, 0}, bgPad);

    Interface::Set_Document("");
}

void Furnace::Right_Click() {
    UI::CustomDocument = "furnace";
    UI::Toggle_Mouse(true);
}