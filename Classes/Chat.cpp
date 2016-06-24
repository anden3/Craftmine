#include "Chat.h"

#include <unicode/ustream.h>

#include "UI.h"
#include "Blocks.h"
#include "Player.h"
#include "Network.h"
#include "Interface.h"
#include "Inventory.h"

#include "main.h"

const double MESSAGE_TIME = 10.0;
const double FADE_TIME = 4.0;
const double CURSOR_BLINK_SPEED = 1.0;
const float SCROLL_AMOUNT = 30.0f;

static glm::vec4 chatDims;
static glm::vec2 chatPad;

void Chat::Init() {
    // Have to wait with initializing UI dimensions, because screen dimensions aren't set at compile time.
    chatDims = glm::vec4(Scale(50, 100), Scale(400, 400));
    chatPad = Scale(10);

    glm::vec2 messageDims = Scale(50, 70);
    glm::vec4 messageArea(Scale(50, 60), Scale(400, 20));

    interface.Set_Document("chatFocused");
    interface.Add_Text("newMessage", "", messageDims);
    interface.Add_Text("cursor", "|", messageDims);
    interface.Get_Text_Element("cursor")->Opacity = 0.0f;

    interface.Add_Background("bgChat", glm::vec4(chatDims.xy() - chatPad, chatDims.zw() + chatPad * 2.0f));
    interface.Add_Background("bgMessage", glm::vec4(messageArea.xy() - chatPad, messageArea.zw() + chatPad * 2.0f));
    interface.Get_Background("bgMessage")->Opacity = 0.8f;

    interface.Set_Document("");
}

void Chat::Write(std::string text) {
    std::vector<std::string> strings = interface.Get_Fitting_String(text, static_cast<int>(chatDims.z));
    int index = 1;

    for (auto const &string : strings) {
        Move_Up((index++ == 1) ? 30 : 20);

        ++MessageCount;
        Messages.emplace(MessageCount, Message(MessageCount, chatDims.y, string, MESSAGE_TIME));

        interface.Set_Document("chat");
        interface.Add_Text(std::to_string(MessageCount), string, chatDims.xy());
        interface.Set_Document("");
    }
}

void Chat::Key_Handler(int key) {
    if (Focused) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                Focused = false;
                FocusToggled = true;

                NewMessage.clear();
                CursorPos = 0;

                Toggle_Cursor(0);
                Update_Message();
                break;

            case GLFW_KEY_BACKSPACE:
                NewMessage.pop_back();
                --CursorPos;

                Update_Message();
                break;

            case GLFW_KEY_ENTER:
                Submit();

                Focused = false;
                FocusToggled = true;
                break;

            case GLFW_KEY_UP:
                Get_Prev();
                break;

            case GLFW_KEY_DOWN:
                Get_Next();
                break;

            case GLFW_KEY_LEFT:
                if (CursorPos > 0) {
                    --CursorPos;
                    Update_Message();
                }
                break;

            case GLFW_KEY_RIGHT:
                if (CursorPos < static_cast<unsigned int>(NewMessage.size())) {
                    ++CursorPos;
                    Update_Message();
                }
                break;
        }
    }
    else {
        switch (key) {
            case GLFW_KEY_T:
                Focused = true;
                FocusToggled = true;
                UI::Toggle_Mouse(1);
                break;
        }
    }
}

void Chat::Mouse_Handler(double x, double y) {
    MouseX = static_cast<float>(x);
    MouseY = static_cast<float>(SCREEN_HEIGHT - y);

    MouseOverChat = false;

    if (MouseX >= chatDims.x - chatPad.x && MouseX <= chatDims.x + chatDims.z + chatPad.x) {
        if (MouseY >= chatDims.y - chatPad.y && MouseY <= chatDims.y + chatDims.w + chatPad.y) {
            MouseOverChat = true;
        }
    }
}

void Chat::Scroll(int direction) {
    if (MouseOverChat) {
        if (Messages.size() == 0) {
            return;
        }

        float offset = -SCROLL_AMOUNT * direction;

        if (Messages[MessageCount].Y + offset > chatDims.y + chatPad.y) {
            return;
        }

        if (Messages[1].Y + offset < chatDims.y + chatDims.w - chatPad.y - 20) {
            return;
        }

        interface.Set_Document("chat");

        for (auto &message : Messages) {
            TextElement* text = interface.Get_Text_Element(std::to_string(message.first));
            message.second.Y += offset;
            text->Y += offset;

            if (message.second.Y >= chatDims.y && message.second.Y <= chatDims.y + chatDims.w) {
                text->Opacity = 1.0f;
                message.second.OutOfView = false;
            }
            else {
                text->Opacity = 0.0f;
                message.second.OutOfView = true;
            }
        }

        interface.Set_Document("");
    }
}

void Chat::Input(unsigned int key) {
    // Stops player from inputting color codes by blocking the ampersand symbol
    if (key == 38) {
        return;
    }

    UnicodeString string(static_cast<UChar32>(key));
    std::string str;
    string.toUTF8String(str);

    NewMessage += str;
    ++CursorPos;

    Update_Message();
}

void Chat::Submit() {
    if (NewMessage.length() > 0) {
        if (NewMessage.front() == '/') {
            for (auto const &message : Process_Commands(NewMessage.substr(1))) {
                Write(message);
            }
        }
        else {
            Write("&4" + std::string(PLAYER_NAME) + ": &f" + NewMessage);
        }

        if (Multiplayer) {
            Client.Send(NewMessage);
        }

        History.push_back(NewMessage);
        HistoryIndex = static_cast<unsigned int>(History.size());

        NewMessage.clear();

        Toggle_Cursor(0);
        Update_Message();
    }
}

void Chat::Toggle_Cursor(float opacity) {
    CursorVisible = !CursorVisible;
    opacity = (opacity == -1.0f) ? CursorVisible : 1.0f;

    interface.Set_Document("chatFocused");
    interface.Get_Text_Element("cursor")->Opacity = opacity;
    interface.Set_Document("");
}

void Chat::Update_Message() {
    interface.Set_Document("chatFocused");

    TextElement* message = interface.Get_Text_Element("newMessage");
    TextElement* cursor = interface.Get_Text_Element("cursor");

    message->Text = NewMessage;
    cursor->X = chatDims.x + interface.Get_String_Width(NewMessage.substr(0, CursorPos));
    message->Mesh();

    interface.Set_Document("");
}

void Chat::Move_Up(float spacing) {
    interface.Set_Document("chat");

    for (auto &message : Messages) {
        message.second.Y += spacing;
        TextElement* text = interface.Get_Text_Element(std::to_string(message.first));
        text->Y = message.second.Y;

        if (message.second.Y >= chatDims.y + chatDims.w) {
            message.second.OutOfView = true;
            text->Opacity = 0.0f;
        }
    }

    interface.Set_Document("");
}

void Chat::Get_Prev() {
    if (HistoryIndex > 0) {
        NewMessage = History[--HistoryIndex];
        Update_Message();
    }
}

void Chat::Get_Next() {
    if (HistoryIndex < History.size() - 1 && History.size() > 0) {
        NewMessage = History[++HistoryIndex];
    }
    else {
        HistoryIndex = static_cast<unsigned int>(History.size());
        NewMessage = "";
    }

    Update_Message();
}

void Chat::Update() {
    interface.Set_Document("chat");

    for (auto &message : Messages) {
        std::string name = std::to_string(message.first);

        if (message.second.OutOfView) {
            continue;
        }

        TextElement* text = interface.Get_Text_Element(name);

        if (FocusToggled) {
            if (Focused) {
                message.second.RealOpacity = text->Opacity;
            }

            text->Opacity = Focused ? 1.0f : message.second.RealOpacity;
        }
        else if (!message.second.Hidden) {
            message.second.TimeLeft -= DeltaTime;

            if (message.second.TimeLeft <= FADE_TIME) {
                if (message.second.TimeLeft <= 0.0) {
                    message.second.Hidden = true;
                }

                message.second.RealOpacity = (message.second.TimeLeft <= 0.0) ?
                    0.0f :
                    static_cast<float>(message.second.TimeLeft / FADE_TIME);

                if (!Focused) {
                    text->Opacity = message.second.RealOpacity;
                }
            }
        }
    }

    interface.Set_Document("");

    if (FocusToggled) {
        UI::Toggle_Mouse(Focused);
    }

    FocusToggled = false;

    if (Focused) {
        if (glfwGetTime() - LastCursorToggle >= CURSOR_BLINK_SPEED) {
            Toggle_Cursor();
            LastCursorToggle = glfwGetTime();
        }

        interface.Draw_Document("chatFocused");
    }

    interface.Draw_Document("chat");
}

std::vector<std::string> Chat::Process_Commands(std::string message) {
    std::vector<std::string> parameters = Split(message, ' ');

    if (parameters.size() == 0) {
        return std::vector<std::string> {"/"};
    }

    std::string command = parameters[0];

    if (command == "help") {
        return std::vector<std::string> {
            "List of commands:",
            "&a/help&f: Displays this list.",
            "&a/tp&f <&2X&f> <&2Y&f> <&2Z&f>: Teleports player to specified location.",
            "&a/give&f <&2ID&f> [&2NUM&f]: Gives &2NUM&f \
                (or 64 if &2NUM&f is unspecified) blocks of type &2ID&f to player.",
            "&a/clear&f: Clears the player's inventory.",
            "&a/gamemode&f <&2MODE&f>: Sets the player's gamemode to &6Creative&f \
                if &2MODE&f is &5'c'&f, or &6Survival&f if &2MODE&f is &5's'&f.",
            "&a/pos&f: Displays player's position, current chunk, and current tile."
        };
    }

    else if (command == "tp") {
        if (parameters.size() < 4) {
            return std::vector<std::string> {"&4Error! &fNot enough parameters."};
        }

        int x = std::stoi(parameters[1]);
        int y = std::stoi(parameters[2]);
        int z = std::stoi(parameters[3]);

        player.Teleport(glm::vec3(x, y, z));

        return std::vector<std::string> {"Player teleported to (&3" + parameters[1] +
            "&f, &3" + parameters[2] + "&f, &3" + parameters[3] + "&f)."};
    }

    else if (command == "give") {
        if (parameters.size() < 2) {
            return std::vector<std::string> {"&4Error! &fMissing parameter <&2ID&f>."};
        }
        else if (parameters.size() > 4) {
            return std::vector<std::string> {"&4Error! &fToo many parameters."};
        }

        std::string name = "";

        bool lastParameterIsSize = false;
        int size = 64;

        if (parameters.size() >= 3) {
            try {
                lastParameterIsSize = true;
                size = std::stoi(parameters[parameters.size() - 1]);
            }
            catch (std::invalid_argument) {
                size = 64;
                lastParameterIsSize = false;
            }
        }

        for (unsigned long p = 1; p < parameters.size() - lastParameterIsSize; ++p) {
            name += parameters[p] + ((p < parameters.size() - (lastParameterIsSize + 1)) ? " " : "");
        }

        int type;
        int data = 0;

        unsigned long delimiterPos = parameters[1].find(':');

        if (delimiterPos != std::string::npos) {
            type = std::stoi(parameters[1].substr(0, delimiterPos));
            data = std::stoi(parameters[1].substr(delimiterPos + 1));

            if (Blocks::Exists(type) && !Blocks::Exists(type, data)) {
                return std::vector<std::string> {"&4Error! &fNo block exists with that data value."};
            }
        }
        else {
            try {
                type = std::stoi(name);
            }
            catch (std::invalid_argument) {
                const Block* block = Blocks::Get_Block_From_Name(name);

                if (block == nullptr) {
                    const Item* item = Blocks::Get_Item_From_Name(name);

                    if (item == nullptr) {
                        return std::vector<std::string> {"&4Error! &fNo block exists with that name."};
                    }

                    name = item->Name;
                    type = item->ID;
                    data = item->Data;
                }
                else {
                    name = block->Name;
                    type = block->ID;
                    data = block->Data;
                }
            }
        }

        if (!Blocks::Exists(type, data)) {
            return std::vector<std::string> {"&4Error! &fNo block exists with that ID."};
        }

        name = Blocks::Get_Name(type, data);
        inventory.Add_Stack(type, data, size);
        player.Mesh_Holding();
        return std::vector<std::string> {"Given &2" + name + " &fto player."};
    }

    else if (command == "clear") {
        inventory.Clear();
        player.Mesh_Holding();
        return std::vector<std::string> {"Inventory cleared!"};
    }

    else if (command == "gamemode") {
        if (parameters.size() < 2) {
            return std::vector<std::string> {"&4Error! &fMissing parameter <&2MODE&f>."};
        }

        if (parameters[1] == "c" || parameters[1] == "s") {
            player.Creative = parameters[1] == "c";
            return std::vector<std::string> {"Gamemode changed to &6" +
                std::string(((parameters[1] == "c") ? "Creative&f." : "Survival&f."))};
        }

        return std::vector<std::string> {"&4Error! %fInvalid gamemode."};
    }

    else if (command == "pos") {
        return std::vector<std::string> {
            "Position: " + Format_Vector(player.WorldPos),
            "Chunk: " + Format_Vector(player.CurrentChunk),
            "Tile: " + Format_Vector(player.CurrentTile)
        };
    }

    return std::vector<std::string> {"&4Error! &fCommand not recognized."};
}
