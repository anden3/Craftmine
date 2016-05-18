#include "Interface.h"

#include <glm/gtc/matrix_transform.hpp>

#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

const std::string FONT = "Roboto";
const int FONT_SIZE = 15;

Shader* UIBackgroundShader;
Shader* UIBorderShader;
Shader* UITextureShader;
Shader* UI3DShader;

Buffer TextBuffer;

int BgColorLoc;
int BgAlphaLoc;
int BorderColorLoc;

const int       TEXT_TEXTURE_UNIT         = 10;
const float     TEXT_PADDING              = 20;

const float     BUTTON_PADDING            = 20.0f;
const float     BUTTON_OPACITY            = 1.0f;
const float     BUTTON_TEXT_OPACITY       = 1.0f;
const glm::vec3 BUTTON_COLOR              = glm::vec3(0.5f);
const glm::vec3 BUTTON_HOVER_COLOR        = glm::vec3(0.7f);
const glm::vec3 BUTTON_CLICK_COLOR        = glm::vec3(0.3f, 0.3f, 0.8f);
const glm::vec3 BUTTON_TEXT_COLOR         = glm::vec3(1.0f);

const float     SLIDER_PADDING            = 20.0f;
const float     SLIDER_WIDTH              = 10.0f;
const float     SLIDER_OPACITY            = 1.0f;
const float     SLIDER_HANDLE_OPACITY     = 1.0f;
const float     SLIDER_TEXT_OPACITY       = 1.0f;
const glm::vec3 SLIDER_COLOR              = glm::vec3(0.2f);
const glm::vec3 SLIDER_HANDLE_COLOR       = glm::vec3(0.5f);
const glm::vec3 SLIDER_HANDLE_HOVER_COLOR = glm::vec3(0.7f);
const glm::vec3 SLIDER_HANDLE_CLICK_COLOR = glm::vec3(0.3f, 0.3f, 0.8f);
const glm::vec3 SLIDER_TEXT_COLOR         = glm::vec3(1.0f);

const float     BACKGROUND_OPACITY        = 0.7f;
const glm::vec3 BACKGROUND_COLOR          = glm::vec3(0.0f);
const glm::vec3 BACKGROUND_BORDER_COLOR   = glm::vec3(0.5f);

Data Get_3D_Mesh(unsigned int type, float x, float y, bool offsets) {
    Data data;
    
    glm::vec2 texPosition = textureCoords[type];
    static float textureStepX = (1.0f / 16.0f);
    static float textureStepY = (1.0f / 32.0f);
    
    float texStartX = textureStepX * (texPosition.x - 1.0f);
    float texStartY = textureStepY * (texPosition.y - 1.0f);
    
    x *= 2.005f;
    y *= 2.005f;
    
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            if (CustomVertices.count(type)) {
                data.push_back(CustomVertices[type][i][vertices[i][j][0]].x);
                data.push_back(CustomVertices[type][i][vertices[i][j][1]].y);
                data.push_back(CustomVertices[type][i][vertices[i][j][2]].z);
            }
            else {
                data.push_back(vertices[i][j][0]);
                data.push_back(vertices[i][j][1]);
                data.push_back(vertices[i][j][2]);
            }
            
            if (CustomTexCoords.count(type)) {
                data.push_back(CustomTexCoords[type][i][tex_coords[i][j][0]].x / 16.0f);
                data.push_back(CustomTexCoords[type][i][tex_coords[i][j][1]].y / 32.0f);
            }
            else if (MultiTextures.count(type)) {
                data.push_back((MultiTextures[type][i].x - 1.0f + tex_coords[i][j][0]) / 16.0f);
                data.push_back((MultiTextures[type][i].y - 1.0f + tex_coords[i][j][1]) / 32.0f);
            }
            else {
                data.push_back(texStartX + tex_coords[i][j][0] / 16.0f);
                data.push_back(texStartY + tex_coords[i][j][1] / 32.0f);
            }
            
            if (offsets) {
                data.push_back(x);
                data.push_back(y);
            }
        }
    }
    
    return data;
}

Data Get_Rect(float x1, float x2, float y1, float y2) {
    return Data {x1, y1, x2, y1, x2, y2, x1, y1, x2, y2, x1, y2};
}

Data Get_Border(float x1, float x2, float y1, float y2) {
    return Data {
        x1 - 0.5f, y1, x2, y1,
        x2, y1, x2, y2,
        x2, y2, x1, y2,
        x1, y2, x1, y1
    };
}

void Extend(Data &storage, const Data input) {
    for (auto const &object : input) {
        storage.push_back(object);
    }
}

struct Character {
    unsigned int TextureID;
    glm::ivec2 Size;
    glm::ivec2 Bearing;
    unsigned int Advance;
};

std::map<char, Character> Characters;

void TextElement::Create(std::string text, float x, float y, float opacity, glm::vec3 color, float scale) {
    Text = text;
    Width = Get_Width();
    
    Text = text;
    X = x;
    Y = y;
    
    Opacity = opacity;
    Scale = scale;
    Color = color;
    
    Width = Get_Width();
}

void TextElement::Center(float x, float y, float width) {
    X = floor(x + (width - Width) / 2);
    Y = floor(y + TEXT_PADDING - float(FONT_SIZE / 6));
}

void TextElement::Draw() {
    if (Opacity == 0.0f) {
        return;
    }
    
    TextBuffer.BufferShader->Upload("textColor", glm::vec4(Color, Opacity));
    
    glActiveTexture(GL_TEXTURE0 + TEXT_TEXTURE_UNIT);
    
    float charX = X;
    
    for (char const c : Text) {
        Character ch = Characters[c];
        
        float xPos = charX + ch.Bearing.x * Scale;
        float yPos = Y - (ch.Size.y - ch.Bearing.y) * Scale;
        
        float w = ch.Size.x * Scale;
        float h = ch.Size.y * Scale;
        
        Data text_vertices = {
            xPos,     yPos + h,   0, 0,
            xPos,     yPos,       0, 1,
            xPos + w, yPos,       1, 1,
            
            xPos,     yPos + h,   0, 0,
            xPos + w, yPos,       1, 1,
            xPos + w, yPos + h,   1, 0
        };
        
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        
        TextBuffer.Upload(text_vertices, 0, true);
        TextBuffer.Draw();
        
        charX += (ch.Advance >> 6) * Scale;
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);
}

float TextElement::Get_Width() {
    float width = 0;
    
    for (char const &c : Text) {
        width += (Characters[c].Advance >> 6) * Scale;
    }
    
    return width;
}

Button::Button(std::string text, float x, float y, float w, Func &function) : X(x), Y(y), Width(w), Function(function) {
    Height = BUTTON_PADDING * 2;
    Opacity = BUTTON_OPACITY;
    Color = BUTTON_COLOR;
    
    Text.Create(text, X, Y);
    Text.Center(X, Y, Width);
    
    Text.Opacity = BUTTON_TEXT_OPACITY;
    Text.Color = BUTTON_TEXT_COLOR;
    
    BackgroundBuffer.Init(UIBackgroundShader);
    BorderBuffer.Init(UIBorderShader);
    
    BackgroundBuffer.Create(std::vector<int> {2}, Data {X, Y + Height, X, Y, X + Width, Y, X, Y + Height, X + Width, Y, X + Width, Y + Height});
    BorderBuffer.Create(std::vector<int> {2}, Data {X, Y, X + Width, Y, X + Width, Y + Height, X, Y + Height});
    
    BorderBuffer.VertexType = GL_LINE_LOOP;
}

inline void Button::Hover() { Color = BUTTON_HOVER_COLOR; }
inline void Button::Stop_Hover() { Color = BUTTON_COLOR; }
inline void Button::Press() { Color = BUTTON_CLICK_COLOR; }
inline void Button::Release() { Color = BUTTON_COLOR; Function(); }

void Button::Draw() {
    UIBackgroundShader->Upload(BgColorLoc, Color);
    UIBackgroundShader->Upload(BgAlphaLoc, Opacity);
    BackgroundBuffer.Draw();
    
    glClear(GL_DEPTH_BUFFER_BIT);
    
    UIBorderShader->Upload(BorderColorLoc, glm::vec3(0));
    BorderBuffer.Draw();
    
    Text.Draw();
}
    
Slider::Slider(std::string text, float x, float y, float w, float min, float max, float value, Func &function) : X(x), Y(y), Width(w), Min(min), Max(max), Value(value), Function(function) {
    Height = SLIDER_PADDING * 2;
    
    Opacity = SLIDER_OPACITY;
    HandleOpacity = SLIDER_HANDLE_OPACITY;
    
    Color = SLIDER_COLOR;
    HandleColor = SLIDER_HANDLE_COLOR;
    
    Text.Create(text, X, Y);
    Text.Center(X, Y, Width);
    
    Text.Opacity = SLIDER_TEXT_OPACITY;
    Text.Color = SLIDER_TEXT_COLOR;
    
    float sw = SLIDER_WIDTH / 2;
    float sx = x + w * ((Value - Min) / (Max - Min));
    HandlePosition = sx;
    
    BackgroundBuffer.Init(UIBackgroundShader);
    HandleBuffer.Init(UIBackgroundShader);
    BorderBuffer.Init(UIBorderShader);
    
    BackgroundBuffer.Create(std::vector<int> {2}, Data {X, Y + Height, X, Y, X + Width, Y, X, Y + Height, X + Width, Y, X + Width, Y + Height});
    BorderBuffer.Create(std::vector<int> {2}, Data {X, Y, X + Width, Y, X + Width, Y + Height, X - 0.5f, Y + Height});
    HandleBuffer.Create(std::vector<int> {2}, Data {sx, Y + Height, sx, Y, sx + sw, Y, sx, Y + Height, sx + sw, Y, sx + sw, Y + Height});
    
    BorderBuffer.VertexType = GL_LINE_LOOP;
}

inline void Slider::Hover() { HandleColor = SLIDER_HANDLE_HOVER_COLOR; }
inline void Slider::Stop_Hover() { HandleColor = SLIDER_HANDLE_COLOR; }
inline void Slider::Press() { HandleColor = SLIDER_HANDLE_CLICK_COLOR; }
inline void Slider::Release() { HandleColor = SLIDER_HANDLE_COLOR; Function(); }
    
void Slider::Move(float position) {
    if (position <= X + (SLIDER_WIDTH / 2)) {
        position = X + (SLIDER_WIDTH / 2);
    }
    else if (position >= X + Width - 1) {
        position = X + Width - 1;
    }
    
    float percentage = (position - X) / Width;
    float x = X + Width * percentage - (SLIDER_WIDTH / 2);
    float w = SLIDER_WIDTH / 2;
    
    HandleBuffer.Upload(Data {x, Y + Height, x, Y, x + w, Y,  x, Y + Height, x + w, Y, x + w, Y + Height});
    
    HandlePosition = x;
    Value = (Max - Min) * percentage;
}

void Slider::Draw() {
    UIBackgroundShader->Upload(BgAlphaLoc, Opacity);
    UIBackgroundShader->Upload(BgColorLoc, Color);
    
    BackgroundBuffer.Draw();
    
    glClear(GL_DEPTH_BUFFER_BIT);
    
    UIBackgroundShader->Upload(BgAlphaLoc, HandleOpacity);
    UIBackgroundShader->Upload(BgColorLoc, HandleColor);
    
    HandleBuffer.Draw();
    
    glClear(GL_DEPTH_BUFFER_BIT);
    
    BorderBuffer.Draw();
    Text.Draw();
}

Background::Background(float x, float y, float w, float h, bool border, glm::vec2 gridWidth) : X(x), Y(y), Width(w), Height(h) {
    Opacity = BACKGROUND_OPACITY;
    Color = BACKGROUND_COLOR;
    
    BackgroundBuffer.Init(UIBackgroundShader);
    BackgroundBuffer.Create(std::vector<int> {2}, Get_Rect(X, X + Width, Y, Y + Height));
    
    if (border) {
        GridSet = true;
        GridBuffer.Init(UIBorderShader);
        
        Data gridData;
        
        if (gridWidth != glm::vec2(0, 0)) {
            GridWidth = gridWidth;
            float paddingX = GridWidth.x / 8.0f;
            float paddingY = GridWidth.y / 8.0f;
            
            for (float gx = X + paddingX; gx <= X + Width - paddingX; gx += GridWidth.x) {
                Extend(gridData, Data {gx, Y + paddingY, gx, Y + Height - paddingY});
            }
            
            for (float gy = Y + paddingY; gy <= Y + Height - paddingY; gy += GridWidth.y) {
                Extend(gridData, Data {X + paddingX, gy, X + Width - paddingX, gy});
            }
        }
        else {
            Extend(gridData, Get_Border(X + 5, X + Width - 5, Y + 5, Y + Height - 5));
        }
        
        GridBuffer.Create(std::vector<int> {2}, gridData);
        GridBuffer.VertexType = GL_LINES;
    }
}

void Background::Move(float dx, float dy, bool absolute) {
    if (absolute) {
        X = dx;
        Y = dy;
    }
    else {
        X += dx;
        Y += dy;
    }
    
    BackgroundBuffer.Upload(Get_Rect(X, X + Width, Y, Y + Height));
    
    if (GridWidth != glm::vec2(0, 0)) {
        Data gridData;
        
        for (float gx = X; gx <= X + Width; gx += GridWidth.x) {
            Extend(gridData, Data {gx, Y, gx, Y + Height});
        }
        
        for (float gy = Y; gy <= Y + Height; gy += GridWidth.y) {
            Extend(gridData, Data {X, gy, X + Width, gy});
        }
        
        GridBuffer.Upload(gridData);
    }
    else if (GridSet) {
        GridBuffer.Upload(Get_Border(X, X + Width, Y, Y + Height));
    }
}

void Background::Draw() {
    UIBackgroundShader->Upload(BgColorLoc, Color);
    UIBackgroundShader->Upload(BgAlphaLoc, Opacity);
    BackgroundBuffer.Draw();
    
    if (GridSet) {
        glClear(GL_DEPTH_BUFFER_BIT);
        GridBuffer.Draw();
    }
}

OrthoElement::OrthoElement(int type, float x, float y, float scale) {
    OrthoBuffer.Init(UI3DShader);
    
    Type = type;
    Scale = scale;
    
    if (type == 0) {
        OrthoBuffer.Create(std::vector<int> {3, 2, 2});
    }
    else {
        OrthoBuffer.Create(std::vector<int> {3, 2, 2}, Get_3D_Mesh(type, x, y, true));
    }
}

void OrthoElement::Mesh(int type, float x, float y) {
    Type = type;
    OrthoBuffer.Upload(Get_3D_Mesh(type, x, y, true));
}

void OrthoElement::Draw() {
    if (Type != 0) {
        UI3DShader->Upload("scale", Scale);
        OrthoBuffer.Draw();
    }
}

enum ActiveElementType { NONE, BUTTON, SLIDER };

int ActiveElement = NONE;
Button* ActiveButton;
Slider* ActiveSlider;

std::map<std::string, std::map<std::string, Button>> Buttons;
std::map<std::string, std::map<std::string, Slider>> Sliders;
std::map<std::string, std::map<std::string, TextElement>> TextElements;
std::map<std::string, std::map<std::string, Background>> Backgrounds;
std::map<std::string, std::map<std::string, OrthoElement>> OrthoElements;

void Interface::Init() {
    Init_Shaders();
    Init_Text();
}

void Interface::Init_Shaders() {
    glActiveTexture(GL_TEXTURE0);
    
    UIBackgroundShader = new Shader("ui");
    UIBorderShader = new Shader("uiBorder");
    UITextureShader = new Shader("uiTex");
    UI3DShader = new Shader("ortho");
    
    BgColorLoc = UIBackgroundShader->Get_Location("color");
    BgAlphaLoc = UIBackgroundShader->Get_Location("alpha");
    BorderColorLoc = UIBorderShader->Get_Location("color");
    
    glm::mat4 model;
    model = glm::rotate(model, glm::radians(20.0f), glm::vec3(1, 0, 0));
    model = glm::rotate(model, 45.0f, glm::vec3(0, 1, 0));
    
    glm::mat4 projection = glm::ortho(0.0f, float(SCREEN_WIDTH), 0.0f, float(SCREEN_HEIGHT));
    
    UIBackgroundShader->Upload("projection", projection);
    
    UIBorderShader->Upload("projection", projection);
    UIBorderShader->Upload(BorderColorLoc, glm::vec3(0));
    
    UITextureShader->Upload("projection", projection);
    UITextureShader->Upload("tex", 0);
    
    UI3DShader->Upload("projection", glm::ortho(0.0f, float(SCREEN_WIDTH), 0.0f, float(SCREEN_HEIGHT), -1000.0f, 1000.0f));
    UI3DShader->Upload("model", model);
    UI3DShader->Upload("tex", 0);
}

void Interface::Init_Text() {
    Shader* TextShader = new Shader("text");
    
    FT_Library ft;
    FT_Face face;
    
    FT_Init_FreeType(&ft);
    FT_New_Face(ft, std::string("fonts/" + FONT + ".ttf").c_str(), 0, &face);
    FT_Set_Pixel_Sizes(face, 0, FONT_SIZE);
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    for (unsigned char c = 0; c < 128; c++) {
        FT_Load_Char(face, c, FT_LOAD_RENDER);
        
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cerr << "ERROR::FREETYPE: Failed to load Glyph " << c << std::endl;
            continue;
        }
        
        unsigned int texture;
        glGenTextures(1, &texture);
        glActiveTexture(GL_TEXTURE0 + TEXT_TEXTURE_UNIT);
        glBindTexture(GL_TEXTURE_2D, texture);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
                     face->glyph->bitmap.width,
                     face->glyph->bitmap.rows,
                     0, GL_RED, GL_UNSIGNED_BYTE,
                     face->glyph->bitmap.buffer
                     );
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            (unsigned int)face->glyph->advance.x
        };
        
        Characters.insert(std::pair<char, Character>(c, character));
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    
    Data data;
    data.resize(4 * 6);
    
    TextBuffer.Init(TextShader);
    TextBuffer.Create(std::vector<int> {4}, data);
    
    TextShader->Upload("projection", glm::ortho(0.0f, (float)SCREEN_WIDTH, 0.0f, (float)SCREEN_HEIGHT));
    TextShader->Upload("text", TEXT_TEXTURE_UNIT);
}

void Interface::Add_Text(std::string name, std::string text, float x, float y) {
    TextElements[ActiveDocument][name] = TextElement();
    TextElements[ActiveDocument][name].Create(text, x, y);
}

void Interface::Add_Button(std::string name, std::string text, float x, float y, float w, Func &function) {
    Buttons[ActiveDocument].emplace(name, Button(text, x, y, w, function));
}

void Interface::Add_Slider(std::string name, std::string text, float x, float y, float w, float min, float max, float value, Func &function) {
    Sliders[ActiveDocument].emplace(name, Slider(text, x, y, w, min, max, value, function));
}

void Interface::Add_Background(std::string name, float x, float y, float w, float h, bool border, glm::vec2 gridWidth) {
    Backgrounds[ActiveDocument].emplace(name, Background(x, y, w, h, border, gridWidth));
}

void Interface::Add_3D_Element(std::string name, int type, float x, float y, float scale) {
    OrthoElements[ActiveDocument].emplace(name, OrthoElement(type, x, y, scale));
}

void Interface::Delete_Text(std::string name) {
    TextElements[ActiveDocument].erase(name);
}
void Interface::Delete_Button(std::string name) {
    Buttons[ActiveDocument].erase(name);
}
void Interface::Delete_Slider(std::string name) {
    Sliders[ActiveDocument].erase(name);
}
void Interface::Delete_Background(std::string name) {
    Backgrounds[ActiveDocument].erase(name);
}
void Interface::Delete_3D_Element(std::string name) {
    OrthoElements[ActiveDocument].erase(name);
}

TextElement* Interface::Get_Text_Element(std::string name) {
    return &TextElements[ActiveDocument][name];
}

Button* Interface::Get_Button(std::string name) {
    return &Buttons[ActiveDocument][name];
}

Slider* Interface::Get_Slider(std::string name) {
    return &Sliders[ActiveDocument][name];
}

Background* Interface::Get_Background(std::string name) {
    return &Backgrounds[ActiveDocument][name];
}

OrthoElement* Interface::Get_3D_Element(std::string name) {
    return &OrthoElements[ActiveDocument][name];
}

void Interface::Mouse_Handler(float x, float y) {
    if (ActiveDocument == "") {
        return;
    }
    
    for (auto &button : Buttons[ActiveDocument]) {
        if (x >= button.second.X && x <= (button.second.X + button.second.Width)) {
            if (y >= button.second.Y && y <= (button.second.Y + button.second.Height)) {
                button.second.Hover();
                ActiveElement = BUTTON;
                ActiveButton = &button.second;
                ActiveSlider = nullptr;
                return;
            }
        }
        
        button.second.Stop_Hover();
    }
    
    for (auto &slider : Sliders[ActiveDocument]) {
        if (x >= slider.second.HandlePosition && x <= (slider.second.HandlePosition + SLIDER_WIDTH)) {
            if (y >= slider.second.Y && y <= (slider.second.Y + slider.second.Height)) {
                slider.second.Hover();
                ActiveElement = SLIDER;
                ActiveSlider = &slider.second;
                ActiveButton = nullptr;
                return;
            }
        }
        
        slider.second.Stop_Hover();
    }
    
    ActiveElement = NONE;
    ActiveSlider = nullptr;
    ActiveButton = nullptr;
}

void Interface::Click(int mouseButton, int action) {
    if (ActiveElement == NONE) {
        return;
    }
    
    else if (ActiveElement == BUTTON) {
        if (action == GLFW_PRESS) {
            ActiveButton->Press();
        }
        else if (action == GLFW_RELEASE) {
            ActiveButton->Release();
        }
    }
    else if (ActiveElement == SLIDER) {
        if (action == GLFW_PRESS) {
            ActiveSlider->Press();
        }
        else if (action == GLFW_RELEASE) {
            ActiveSlider->Release();
        }
    }
}

void Interface::Draw_Document(std::string document) {
    for (auto &bg : Backgrounds[document]) {
        bg.second.Draw();
        glClear(GL_DEPTH_BUFFER_BIT);
    }
    
    for (auto &button : Buttons[document]) {
        button.second.Draw();
    }
    
    for (auto &slider : Sliders[document]) {
        slider.second.Draw();
    }
    
    for (auto &object : OrthoElements[document]) {
        object.second.Draw();
    }
    
    glClear(GL_DEPTH_BUFFER_BIT);
    
    for (auto &text : TextElements[document]) {
        text.second.Draw();
    }
}

float Interface::Get_String_Width(std::string string) {
    float currentWidth = 0;
    
    for (char const &c : string) {
        currentWidth += (Characters[c].Advance >> 6);
    }
    
    return currentWidth;
}

int Interface::Get_Fitting_String(std::string string, int width) {
    float currentWidth = 0;
    int index = 0;
    
    for (char const &c : string) {
        currentWidth += (Characters[c].Advance >> 6);
        
        if (currentWidth > width) {
            return index;
        }
        ++index;
    }
    
    return 0;
}