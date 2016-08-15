#include "Furnace.h"

#include "UI.h"
#include "Chat.h"
#include "main.h"
#include "Interface.h"

void Furnace::Init() {
    Data arrow = {
        0, 0, 5, 0, 5, 20,
        0, 0, 5, 20, 0, 20,
        -5, 0, 2.5, -7.5, 10, 0
    };
    
    glm::vec2 bgPad = Scale(10);
    glm::vec2 slotWidth = Scale(80);

    glm::vec4 bgDims = glm::vec4(Scale(220, 400), Scale(160, 180));

    Interface::Set_Document("furnace");
        Interface::Add_Background("bg", glm::vec4(bgDims.xy(), bgDims.zw()), true, {0, 0}, bgPad);

        Interface::Add_Slot("slot1", Scale(280, 430), Scale_Y(40));
        Interface::Add_Slot("slot2", Scale(280, 520), Scale_Y(40));
        
        Interface::Add_Custom("arrow", Scale_X(300), Scale_Y(495), arrow);
        Interface::Get_Custom("arrow")->Color = glm::vec3(1);
    Interface::Set_Document("");
}

void Furnace::Right_Click() {
    UI::CustomDocument = "furnace";
    UI::Toggle_Mouse(true);
}