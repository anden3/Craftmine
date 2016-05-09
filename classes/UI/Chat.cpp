#include "Chat.h"

#include <vector>

#include "Text.h"

#include <unicode/ustream.h>

#ifdef __APPLE__
using UnicodeString;
#endif

const double MESSAGE_TIME = 10.0;
const double FADE_TIME = 4.0;
const double CURSOR_BLINK_SPEED = 1.0;

const float START_X = 50.0f;
const float END_X = 300.0f;

const float START_Y = 100.0f;
const float END_Y = 500.0f;

const float MESSAGE_Y = 70.0f;

const float SPACING = 22.0f;

const glm::vec3 BACKGROUND_COLOR = glm::vec3(0.0f);

const float BACKGROUND_OPACITY = 0.5f;
const float MESSAGE_BOX_OPACITY = 0.7f;

const float CHAT_PAD = 10.0f;

const std::string TEXT_GROUP = "chat";

Chat::Chat() {
    Text::Set_X(TEXT_GROUP, START_X);
}

void Chat::Init(Shader& ui, Shader& uiBorder, unsigned int colorLoc, unsigned int alphaLoc) {
    UIShader = &ui;
    UIBorderShader = &uiBorder;
    
    ColorLocation = colorLoc;
    AlphaLocation = alphaLoc;
    
    Text::Set_Group(TEXT_GROUP);
    Text::Add("newMessage", "", MESSAGE_Y);
    Text::Add("cursor", "|", MESSAGE_Y);
    Text::Set_Opacity("cursor", 0);
    Text::Unset_Group();
    
    Init_Chat_Background();
}

void Chat::Init_Chat_Background() {
    glGenBuffers(1, &BackgroundVBO);
    glGenVertexArrays(1, &BackgroundVAO);
    
    glGenBuffers(1, &MessageVBO);
    glGenVertexArrays(1, &MessageVAO);
    
    std::vector<float> bgData {
        START_X - CHAT_PAD,  START_Y - CHAT_PAD,
        END_X   + CHAT_PAD,  START_Y - CHAT_PAD,
        END_X   + CHAT_PAD,  END_Y   + CHAT_PAD,
        START_X - CHAT_PAD,  START_Y - CHAT_PAD,
        END_X   + CHAT_PAD,  END_Y   + CHAT_PAD,
        START_X - CHAT_PAD,  END_Y   + CHAT_PAD
    };
    
    std::vector<float> messageData {
        START_X - CHAT_PAD,  MESSAGE_Y - CHAT_PAD,
        END_X   + CHAT_PAD,  MESSAGE_Y - CHAT_PAD,
        END_X   + CHAT_PAD,  START_Y   + CHAT_PAD,
        START_X - CHAT_PAD,  MESSAGE_Y - CHAT_PAD,
        END_X   + CHAT_PAD,  START_Y   + CHAT_PAD,
        START_X - CHAT_PAD,  START_Y   + CHAT_PAD
    };
    
    glBindVertexArray(BackgroundVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, BackgroundVBO);
    glBufferData(GL_ARRAY_BUFFER, bgData.size() * sizeof(float), bgData.data(), GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, false, 2 * sizeof(float), (void*)0);
    
    glBindVertexArray(MessageVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, MessageVBO);
    glBufferData(GL_ARRAY_BUFFER, messageData.size() * sizeof(float), messageData.data(), GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, false, 2 * sizeof(float), (void*)0);
    
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Chat::Write(std::string text) {
    if (Text::Get_String_Width(text) > END_X - START_X) {
        std::string fittedWidth = Text::Get_String_To_Width(text, END_X - START_X);
        std::string remainder = text = text.substr(fittedWidth.length() - 1);
        
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
    
    Text::Set_Group(TEXT_GROUP);
    Text::Add(std::to_string(message.ID), text, START_Y);
    Text::Unset_Group();
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
    
    Text::Set_Group(TEXT_GROUP);
    Text::Set_Opacity("cursor", float(opacity));
    Text::Unset_Group();
}

void Chat::Update_Message() {
    Text::Set_Group(TEXT_GROUP);
    Text::Set_Text("newMessage", NewMessage);
    Text::Set_X("cursor", START_X + Text::Get_String_Width(NewMessage.substr(0, CursorPos)));
    Text::Unset_Group();
}

void Chat::Move_Up() {
    Text::Set_Group(TEXT_GROUP);
    
    auto it = std::begin(Messages);
    
    while (it != std::end(Messages)) {
        it->second.Y += SPACING;
        
        if (it->second.Y >= END_Y) {
            Text::Remove(std::to_string(it->first));
            it = Messages.erase(it);
        }
        else {
            Text::Set_Y(std::to_string(it->first), it->second.Y);
        }
        ++it;
    }
    
    Text::Unset_Group();
}

void Chat::Get_Prev() {
    if (HistoryIndex > 0) {
        NewMessage = History[--HistoryIndex];
        Update_Message();
    }
}

void Chat::Get_Next() {
    if (HistoryIndex < History.size() - 1) {
        NewMessage = History[++HistoryIndex];
        Update_Message();
    }
    else {
        HistoryIndex = int(History.size());
        NewMessage = "";
        Update_Message();
    }
}

void Chat::Draw_Background() {
    UIShader->Bind();
    
    glUniform3f(ColorLocation, BACKGROUND_COLOR.r, BACKGROUND_COLOR.g, BACKGROUND_COLOR.b);
    glUniform1f(AlphaLocation, BACKGROUND_OPACITY);
    
    glBindVertexArray(BackgroundVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    
    glUniform1f(AlphaLocation, MESSAGE_BOX_OPACITY);
    
    glBindVertexArray(MessageVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    
    UIShader->Unbind();
}

void Chat::Update() {
    auto it = std::begin(Messages);
    
    Text::Set_Group(TEXT_GROUP);
    
    while (it != std::end(Messages)) {
        if (FocusToggled) {
            if (Focused) {
                it->second.RealOpacity = Text::Get_Opacity(std::to_string(it->first));
                Text::Set_Opacity(std::to_string(it->first), 1.0f);
            }
            else {
                Text::Set_Opacity(std::to_string(it->first), it->second.RealOpacity);
            }
        }
        else if (!it->second.Hidden) {
            it->second.TimeLeft -= DeltaTime;
            
            if (it->second.TimeLeft <= 0.0) {
                it->second.Hidden = true;
                it->second.RealOpacity = 0.0f;
                
                if (!Focused) {
                    Text::Set_Opacity(std::to_string(it->first), 0.0f);
                }
            }
            else if (it->second.TimeLeft <= FADE_TIME) {
                it->second.RealOpacity = float(it->second.TimeLeft / FADE_TIME);
                
                if (!Focused) {
                    Text::Set_Opacity(std::to_string(it->first), it->second.RealOpacity);
                }
            }
        }
        
        ++it;
    }
    
    Text::Unset_Group();
    
    if (FocusToggled) {
        FocusToggled = false;
    }
    
    if (Focused) {
        if (glfwGetTime() - LastCursorToggle >= CURSOR_BLINK_SPEED) {
            Toggle_Cursor();
            LastCursorToggle = glfwGetTime();
        }
        
        Draw_Background();
    }
    
    glClear(GL_DEPTH_BUFFER_BIT);
    Text::Draw_Group(TEXT_GROUP);
}