#include "Chat.h"

#include <unicode/ustream.h>

const double MESSAGE_TIME = 10.0;
const double FADE_TIME = 4.0;
const double CURSOR_BLINK_SPEED = 1.0;

const float START_X = 50.0f;
const float END_X = 300.0f;

const float START_Y = 100.0f;
const float END_Y = 500.0f;

const float MESSAGE_Y = 70.0f;

const float SPACING = 22.0f;
const float CHAT_PAD = 10.0f;

void Chat::Init() {
    interface.Set_Document("chatFocused");
    interface.Add_Text("newMessage", "", START_X, MESSAGE_Y);
    interface.Add_Text("cursor", "", START_X, MESSAGE_Y);
    interface.Get_Text_Element("cursor")->Opacity = 0.0f;
    
    interface.Add_Background("bgChat", START_X - CHAT_PAD, START_Y - CHAT_PAD, (END_X - START_X) + CHAT_PAD * 2, (END_Y - START_Y) + CHAT_PAD * 2);
    interface.Add_Background("bgMessage", START_X - CHAT_PAD, MESSAGE_Y - CHAT_PAD, (END_X - START_X) + CHAT_PAD * 2, (START_Y - MESSAGE_Y) + CHAT_PAD * 2);
    interface.Set_Document("");
}

void Chat::Write(std::string text) {
    int stringPart = interface.Get_Fitting_String(text, END_X - START_X);
    
    if (stringPart > 0) {
        std::string fittedWidth = text.substr(0, stringPart);
        std::string remainder = text.substr(fittedWidth.length() - 1);
        
        Write(fittedWidth);
        Write(remainder);
        return;
    }
    
    Move_Up();
    
    Message message;
    message.ID = ++MessageCount;
    message.Y = START_Y;
    message.Text = text;
    message.TimeLeft = MESSAGE_TIME;
    
    Messages[message.ID] = message;
    
    interface.Set_Document("chat");
    interface.Add_Text(std::to_string(message.ID), text, START_X, START_Y);
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
            Write(Process_Commands(NewMessage.substr(1)));
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
    
    if (opacity == -1) {
        opacity = int(CursorVisible);
    }
    else {
        CursorVisible = opacity == 1;
    }
    
    interface.Set_Document("chatFocused");
    interface.Get_Text_Element("cursor")->Opacity = float(opacity);
    interface.Set_Document("");
}

void Chat::Update_Message() {
    interface.Set_Document("chatFocused");
    interface.Get_Text_Element("newMessage")->Text = NewMessage;
    interface.Get_Text_Element("cursor")->X = START_X + interface.Get_String_Width(NewMessage.substr(0, CursorPos));
    interface.Set_Document("");
}

void Chat::Move_Up() {
    interface.Set_Document("chat");
    
    auto it = std::begin(Messages);
    
    while (it != std::end(Messages)) {
        it->second.Y += SPACING;
        
        if (it->second.Y >= END_Y) {
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
        Update_Message();
    }
    else {
        HistoryIndex = int(History.size());
        NewMessage = "";
        Update_Message();
    }
}

void Chat::Update() {
    auto it = std::begin(Messages);
    
    interface.Set_Document("chat");
    
    while (it != std::end(Messages)) {
        if (FocusToggled) {
            if (Focused) {
                it->second.RealOpacity = interface.Get_Text_Element(std::to_string(it->first))->Opacity;
                interface.Get_Text_Element(std::to_string(it->first))->Opacity = 1.0f;
            }
            else {
                interface.Get_Text_Element(std::to_string(it->first))->Opacity = it->second.RealOpacity;
            }
        }
        else if (!it->second.Hidden) {
            it->second.TimeLeft -= DeltaTime;
            
            if (it->second.TimeLeft <= 0.0) {
                it->second.Hidden = true;
                it->second.RealOpacity = 0.0f;
                
                if (!Focused) {
                    interface.Get_Text_Element(std::to_string(it->first))->Opacity = 0.0f;
                }
            }
            else if (it->second.TimeLeft <= FADE_TIME) {
                it->second.RealOpacity = float(it->second.TimeLeft / FADE_TIME);
                
                if (!Focused) {
                    interface.Get_Text_Element(std::to_string(it->first))->Opacity = it->second.RealOpacity;
                }
            }
        }
        
        ++it;
    }
    
    interface.Set_Document("");
    
    if (FocusToggled) {
        FocusToggled = false;
    }
    
    if (Focused) {
        if (glfwGetTime() - LastCursorToggle >= CURSOR_BLINK_SPEED) {
            Toggle_Cursor();
            LastCursorToggle = glfwGetTime();
        }
        interface.Draw_Document("chatFocused");
    }
    
    glClear(GL_DEPTH_BUFFER_BIT);
    interface.Draw_Document("chat");
}