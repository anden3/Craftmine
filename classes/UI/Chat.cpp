#include "Chat.h"

#include <vector>

#include "Text.h"

#include <unicode/ustream.h>
#pragma comment(lib, "icuio.lib")
#pragma comment(lib, "icuuc.lib")

const double MESSAGE_TIME = 10.0;
const double FADE_TIME = 4.0;

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

void Chat::Input(unsigned int key) {
    switch (key) {
        case 0x001B:
            NewMessage.clear();
            break;
            
        case 0x0008:
            NewMessage.pop_back();
            break;
            
        case 0x000D:
            Write(NewMessage);
            NewMessage.clear();
            break;
            
        default:
            UnicodeString::UnicodeString string((UChar32)key);
            std::string str;
            string.toUTF8String(str);
            
            NewMessage += str;
    }
    
    Update_Message();
}

void Chat::Update_Message() {
    Text::Set_Group(TEXT_GROUP);
    Text::Set_Text("newMessage", NewMessage);
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
        Draw_Background();
    }
    
    glClear(GL_DEPTH_BUFFER_BIT);
    Text::Draw_Group(TEXT_GROUP);
}