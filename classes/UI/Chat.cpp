#include "Chat.h"

#include <GLFW/glfw3.h>
#include <unicode/ustream.h>

#include "Player.h"
#include "Interface.h"
#include "Inventory.h"

const double MESSAGE_TIME = 10.0;
const double FADE_TIME = 4.0;
const double CURSOR_BLINK_SPEED = 1.0;

glm::vec4 chatDims;

void Chat::Init() {
    glm::vec2 messageDims = Scale(50, 70);
    glm::vec2 chatPad = Scale(10);
    
    chatDims = glm::vec4(Scale(50, 100), Scale(250, 400));
    glm::vec4 messageArea(Scale(50, 60), Scale(250, 20));
    
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
    int stringPart = interface.Get_Fitting_String(text, chatDims.z);
    
    if (stringPart > 0) {
        std::string fittedWidth = text.substr(0, stringPart);
        std::string remainder = text.substr(fittedWidth.length() - 1);
        
        Write(fittedWidth);
        Write(remainder);
        return;
    }
    
    Move_Up();
    
    ++MessageCount;
    Messages.emplace(MessageCount, Message(MessageCount, chatDims.y, text, MESSAGE_TIME));
    
    interface.Set_Document("chat");
    interface.Add_Text(std::to_string(MessageCount), text, chatDims.xy());
    interface.Set_Document("");
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
                if (CursorPos < int(NewMessage.size())) {
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
                break;
        }
    }
}

void Chat::Input(unsigned int key) {
    UnicodeString string((UChar32)key);
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
            Write(NewMessage);
        }
        
        History.push_back(NewMessage);
        HistoryIndex = int(History.size());
        
        NewMessage.clear();
        
        Toggle_Cursor(0);
        Update_Message();
    }
}

void Chat::Toggle_Cursor(int opacity) {
    CursorVisible = !CursorVisible;
    opacity = (opacity == -1) ? int(CursorVisible) : 1;
    
    interface.Set_Document("chatFocused");
    interface.Get_Text_Element("cursor")->Opacity = float(opacity);
    interface.Set_Document("");
}

void Chat::Update_Message() {
    interface.Set_Document("chatFocused");
    interface.Get_Text_Element("newMessage")->Text = NewMessage;
    interface.Get_Text_Element("cursor")->X = chatDims.x + interface.Get_String_Width(NewMessage.substr(0, CursorPos));
    interface.Set_Document("");
}

void Chat::Move_Up() {
    static float SPACING = Scale_Y(22);
    
    interface.Set_Document("chat");
    
    auto it = std::begin(Messages);
    
    while (it != std::end(Messages)) {
        it->second.Y += SPACING;
        
        if (it->second.Y >= (chatDims.y + chatDims.w)) {
            interface.Delete_Text(std::to_string(it->first));
            it = Messages.erase(it);
        }
        else {
            interface.Get_Text_Element(std::to_string(it->first))->Y = it->second.Y;
        }
        
        ++it;
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
        HistoryIndex = int(History.size());
        NewMessage = "";
    }
    
    Update_Message();
}

void Chat::Update() {
    auto it = std::begin(Messages);
    
    interface.Set_Document("chat");
    
    while (it != std::end(Messages)) {
        std::string name = std::to_string(it->first);
        Message& message = it->second;
        
        if (FocusToggled) {
            if (Focused) {
                message.RealOpacity = interface.Get_Text_Element(name)->Opacity;
            }
            
            interface.Get_Text_Element(name)->Opacity = Focused ? 1.0f : message.RealOpacity;
        }
        else if (!message.Hidden) {
            message.TimeLeft -= DeltaTime;
            
            if (message.TimeLeft <= FADE_TIME) {
                if (message.TimeLeft <= 0.0) {
                    message.Hidden = true;
                }
                
                message.RealOpacity = (message.TimeLeft <= 0.0) ? 0.0f : float(message.TimeLeft / FADE_TIME);
                
                if (!Focused) {
                    interface.Get_Text_Element(name)->Opacity = message.RealOpacity;
                }
            }
        }
        
        ++it;
    }
    
    interface.Set_Document("");
    
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
    
    if (command == "tp") {
        if (parameters.size() < 4) {
            return std::vector<std::string> {"Error! Not enough parameters."};
        }
        
        int x = std::stoi(parameters[1]);
        int y = std::stoi(parameters[2]);
        int z = std::stoi(parameters[3]);
        
        player.Teleport(glm::vec3(x, y, z));
        
        return std::vector<std::string> {"Player teleported to (" + parameters[1] + ", " + parameters[2] + ", " + parameters[3] + ")."};
    }
    
    else if (command == "give") {
        if (parameters.size() < 2) {
            return std::vector<std::string> {"Error! Missing parameter <BlockID>."};
        }
        
        int block = std::stoi(parameters[1]);
        int size = 64;
        
        if (parameters.size() >= 3) {
            size = std::stoi(parameters[2]);
        }
        
        inventory.Add_Stack(block, size);
        player.Mesh_Holding();
        return std::vector<std::string> {"Given block " + parameters[1] + " to player."};
    }
    
    else if (command == "clear") {
        inventory.Clear();
        player.Mesh_Holding();
        return std::vector<std::string> {"Inventory cleared!"};
    }
    
    else if (command == "gamemode") {
        if (parameters.size() < 2) {
            return std::vector<std::string> {"Error! Missing parameter <mode>."};
        }
        
        if (parameters[1] == "c" || parameters[1] == "s") {
            player.Creative = parameters[1] == "c";
            return std::vector<std::string> {"Gamemode changed to " + std::string(((parameters[1] == "c") ? "Creative." : "Survival."))};
        }
        
        return std::vector<std::string> {"Error! Invalid gamemode."};
    }
    
    else if (command == "pos") {
        return std::vector<std::string> {
            "Position: " + Format_Vector(player.WorldPos),
            "Chunk: " + Format_Vector(player.CurrentChunk),
            "Tile: " + Format_Vector(player.CurrentTile)
        };
    }
    
    return std::vector<std::string> {"Error! Command not recognized."};
}