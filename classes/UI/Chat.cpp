#include "Chat.h"

#include "Interface.h"

#include <unicode/ustream.h>

#include <GLFW/glfw3.h>

const double MESSAGE_TIME = 10.0;
const double FADE_TIME = 4.0;
const double CURSOR_BLINK_SPEED = 1.0;

glm::vec4 chatDims;

void Chat::Init() {
    chatDims = glm::vec4(X_Frac(5, 144), Y_Frac(1, 9), X_Frac(25, 144), Y_Frac(4, 9));
    glm::vec4 messageArea(X_Frac(5, 144), Y_Frac(7, 90), X_Frac(25, 144), Y_Frac(1, 30));
    
    glm::vec2 messageDims(X_Frac(5, 144), Y_Frac(7, 90));
    glm::vec2 chatPad(X_Frac(1, 144), Y_Frac(1, 90));
    
    interface.Set_Document("chatFocused");
    interface.Add_Text("newMessage", "", messageDims);
    interface.Add_Text("cursor", "|", messageDims);
    interface.Get_Text_Element("cursor")->Opacity = 0.0f;
    
    interface.Add_Background("bgChat", glm::vec4(chatDims.xy() - chatPad, chatDims.zw() + chatPad * 2.0f));
    interface.Add_Background("bgMessage", glm::vec4(messageArea.xy() - chatPad, messageArea.zw() + chatPad * 2.0f));
    
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
                CursorPos--;
                
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
                    CursorPos--;
                    Update_Message();
                }
                break;
                
            case GLFW_KEY_RIGHT:
                if (CursorPos < int(NewMessage.size())) {
                    CursorPos++;
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
    CursorPos++;
    
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
    static float SPACING = Y_Frac(11, 450);
    
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