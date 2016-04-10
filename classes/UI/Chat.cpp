#include "Chat.h"

#include <vector>

#include "Text.h"

const double MESSAGE_TIME = 10.0;
const double FADE_TIME = 4.0;

const float START_X = 50.0f;
const float END_X = 300.0f;

const float START_Y = 100.0f;
const float END_Y = 500.0f;

const float SPACING = 20.0f;

const glm::vec3 BACKGROUND_COLOR = glm::vec3(0.0f);
const float BACKGROUND_OPACITY = 0.5f;

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
    
    Init_Chat_Background();
}

void Chat::Init_Chat_Background() {
    glGenBuffers(1, &BackgroundVBO);
    glGenVertexArrays(1, &BackgroundVAO);
    
    std::vector<float> data {
        START_X - CHAT_PAD,  START_Y - CHAT_PAD,
        END_X   + CHAT_PAD,  START_Y - CHAT_PAD,
        END_X   + CHAT_PAD,  END_Y   + CHAT_PAD,
        START_X - CHAT_PAD,  START_Y - CHAT_PAD,
        END_X   + CHAT_PAD,  END_Y   + CHAT_PAD,
        START_X - CHAT_PAD,  END_Y   + CHAT_PAD
    };
    
    glBindVertexArray(BackgroundVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, BackgroundVBO);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, false, 2 * sizeof(float), (void*)0);
    
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Chat::Write(std::string text) {
    Message message;
    message.ID = ++MessageCount;
    message.Y = START_Y;
    message.Text = text;
    message.TimeLeft = MESSAGE_TIME;
    
    Messages[message.ID] = message;
    
    Text::Set_Group(TEXT_GROUP);
    Text::Add(std::to_string(message.ID), text, START_Y);
    Text::Unset_Group();
    
    Move_Up();
}

void Chat::Input(int key) {
    
}

void Chat::Remove(unsigned int id) {
    Text::Remove(std::to_string(id));
}

void Chat::Move_Up() {
    Text::Set_Group(TEXT_GROUP);
    
    auto it = std::begin(Messages);
    
    while (it != std::end(Messages)) {
        if (it->first != MessageCount) {
            it->second.Y += SPACING;
            
            if (it->second.Y > END_Y) {
                Remove(it->first);
                it = Messages.erase(it);
            }
            else {
                Text::Set_Y(std::to_string(it->first), it->second.Y);
            }
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
    
    UIShader->Unbind();
}

void Chat::Update() {
    auto it = std::begin(Messages);
    
    Text::Set_Group(TEXT_GROUP);
    
    while (it != std::end(Messages)) {
        if (FocusToggled) {
            if (Focused) {
                it->second.OldOpacity = Text::Get_Opacity(std::to_string(it->first));
                Text::Set_Opacity(std::to_string(it->first), 1.0f);
            }
            else {
                Text::Set_Opacity(std::to_string(it->first), it->second.OldOpacity);
            }
        }
        else if (!it->second.Hidden && !Focused) {
            it->second.TimeLeft -= DeltaTime;
            
            if (it->second.TimeLeft <= 0.0) {
                it->second.Hidden = true;
                Text::Set_Opacity(std::to_string(it->first), 0.0f);
            }
            else {
                if (it->second.TimeLeft <= FADE_TIME) {
                    Text::Set_Opacity(std::to_string(it->first), it->second.TimeLeft / FADE_TIME);
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