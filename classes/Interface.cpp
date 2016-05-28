#include "Interface.h"

#include <glm/gtc/matrix_transform.hpp>

#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

#include <SOIL/SOIL.h>

#include "Shader.h"

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
int TextColorLoc;

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
const glm::vec3 SLIDER_COLOR              = glm::vec3(0.5f);
const glm::vec3 SLIDER_HANDLE_COLOR       = glm::vec3(0.7f);
const glm::vec3 SLIDER_HANDLE_HOVER_COLOR = glm::vec3(0.9f);
const glm::vec3 SLIDER_HANDLE_CLICK_COLOR = glm::vec3(0.3f, 0.3f, 0.8f);
const glm::vec3 SLIDER_TEXT_COLOR         = glm::vec3(1.0f);

const float     BAR_PADDING               = 20.0f;
const float     BAR_OPACITY               = 1.0f;
const float     BAR_TEXT_OPACITY          = 1.0f;
const float     BAR_BACKGROUND_OPACITY    = 1.0f;
const glm::vec3 BAR_BACKGROUND_COLOR      = glm::vec3(0.0f, 1.0f, 0.0f);
const glm::vec3 BAR_COLOR                 = glm::vec3(0.2f);
const glm::vec3 BAR_TEXT_COLOR            = glm::vec3(1.0f);

const float     BACKGROUND_OPACITY        = 0.7f;
const glm::vec3 BACKGROUND_COLOR          = glm::vec3(0.0f);
const glm::vec3 BACKGROUND_BORDER_COLOR   = glm::vec3(0.5f);

std::map<char, glm::vec3> ColorCodes = {
    {'0', glm::vec3(0.000, 0.000, 0.000)}, // Black
    {'1', glm::vec3(0.000, 0.000, 0.666)}, // Dark Blue
    {'2', glm::vec3(0.000, 0.666, 0.000)}, // Dark Green
    {'3', glm::vec3(0.000, 0.666, 0.666)}, // Dark Aqua
    {'4', glm::vec3(0.666, 0.000, 0.000)}, // Dark Red
    {'5', glm::vec3(0.666, 0.000, 0.666)}, // Dark Purple
    {'6', glm::vec3(1.000, 0.666, 0.000)}, // Gold
    {'7', glm::vec3(0.666, 0.666, 0.666)}, // Grey
    {'8', glm::vec3(0.333, 0.333, 0.333)}, // Dark Grey
    {'9', glm::vec3(0.333, 0.333, 1.000)}, // Blue
    {'a', glm::vec3(0.333, 1.000, 0.333)}, // Green
    {'b', glm::vec3(0.333, 1.000, 1.000)}, // Aqua
    {'c', glm::vec3(1.000, 0.333, 0.333)}, // Red
    {'d', glm::vec3(1.000, 0.333, 1.000)}, // Light Purple
    {'e', glm::vec3(1.000, 1.000, 0.333)}, // Yellow
    {'f', glm::vec3(1.000, 1.000, 1.000)}, // White
};

struct Character {
    unsigned int TextureID;
    glm::ivec2 Size;
    glm::ivec2 Bearing;
    unsigned int Advance;
};

std::map<char, Character> Characters;

Data Get_3D_Mesh(unsigned int type, float x, float y, bool offsets) {
    Data data;
    
    glm::vec2 texPosition = textureCoords[type];
    
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
                data.push_back(CustomTexCoords[type][i][tex_coords[i][j][0]].x / IMAGE_SIZE_X);
                data.push_back(CustomTexCoords[type][i][tex_coords[i][j][1]].y / IMAGE_SIZE_Y);
            }
            else if (MultiTextures.count(type)) {
                data.push_back((MultiTextures[type][i].x - 1.0f + tex_coords[i][j][0]) / IMAGE_SIZE_X);
                data.push_back((MultiTextures[type][i].y - 1.0f + tex_coords[i][j][1]) / IMAGE_SIZE_Y);
            }
            else {
                data.push_back((texPosition.x - 1.0f + tex_coords[i][j][0]) / IMAGE_SIZE_X);
                data.push_back((texPosition.y - 1.0f + tex_coords[i][j][1]) / IMAGE_SIZE_Y);
            }
            
            if (offsets) {
                data.push_back(x);
                data.push_back(y);
            }
        }
    }
    
    return data;
}

std::tuple<unsigned int, int, int> Load_Texture(std::string file) {
    std::string path = "images/" + file;
    
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    int width, height;
    unsigned char* image = SOIL_load_image(path.c_str(), &width, &height, 0, SOIL_LOAD_RGBA);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glBindTexture(GL_TEXTURE_2D, 0);
    SOIL_free_image_data(image);
    
    return std::make_tuple(texture, width, height);
}

void Take_Screenshot() {
    SOIL_save_screenshot("/Users/mac/Desktop/screenshot.bmp", SOIL_SAVE_TYPE_BMP, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
}

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
    if (Opacity == 0.0f || Text == "") {
        return;
    }
    
    TextBuffer.BufferShader->Upload(TextColorLoc, glm::vec4(Color, Opacity));
    
    glActiveTexture(GL_TEXTURE0 + TEXT_TEXTURE_UNIT);
    
    float charX = X;
    bool nextCharIsControl = false;
    
    for (auto const &c : Text) {
        if (c == '&') {
            nextCharIsControl = true;
        }
        
        else if (nextCharIsControl) {
            nextCharIsControl = false;
            TextBuffer.BufferShader->Upload(TextColorLoc, glm::vec4(ColorCodes[c], Opacity));
        }
        
        else {
            Character ch = Characters[c];
            
            float xPos = charX + ch.Bearing.x * Scale;
            float yPos = Y - (ch.Size.y - ch.Bearing.y) * Scale;
            
            float w = ch.Size.x * Scale;
            float h = ch.Size.y * Scale;
            
            Data text_vertices = { xPos, yPos + h, 0, 0, xPos, yPos, 0, 1, xPos + w, yPos, 1, 1, xPos, yPos + h, 0, 0, xPos + w, yPos, 1, 1, xPos + w, yPos + h, 1, 0 };
            
            glBindTexture(GL_TEXTURE_2D, ch.TextureID);
            
            TextBuffer.Upload(text_vertices, 0, true);
            TextBuffer.Draw();
            
            charX += (ch.Advance >> 6) * Scale;
        }
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

void UIElement::Draw() {
    UIBackgroundShader->Upload(BgAlphaLoc, Opacity);
    UIBackgroundShader->Upload(BgColorLoc, Color);
    
    BackgroundBuffer.Draw();
    
    UIBorderShader->Upload(BorderColorLoc, glm::vec3(0));
    BorderBuffer.Draw();
    
    Text.Draw();
}

Button::Button(std::string text, float x, float y, float w, float h, Func &function) {
    Height = (Height != 0) ? h : BUTTON_PADDING * 2;
    
    X = x;
    Y = y;
    Width = w;
    
    Opacity = BUTTON_OPACITY;
    Color = BUTTON_COLOR;
    
    Function = function;
    
    Text.Create(text, X, Y);
    Text.Center(X, Y, Width);
    
    Text.Opacity = BUTTON_TEXT_OPACITY;
    Text.Color = BUTTON_TEXT_COLOR;
    
    BackgroundBuffer.Init(UIBackgroundShader);
    BorderBuffer.Init(UIBorderShader);
    
    BackgroundBuffer.Create(2, Data {X, Y + Height, X, Y, X + Width, Y, X, Y + Height, X + Width, Y, X + Width, Y + Height});
    BorderBuffer.Create(2, Data {X, Y, X + Width, Y, X + Width, Y + Height, X, Y + Height});
    
    BorderBuffer.VertexType = GL_LINE_LOOP;
}

inline void Button::Hover() { Color = BUTTON_HOVER_COLOR; }
inline void Button::Stop_Hover() { Color = BUTTON_COLOR; }
inline void Button::Press() { Color = BUTTON_CLICK_COLOR; }
inline void Button::Release() { Color = BUTTON_COLOR; Function(); }
    
Slider::Slider(std::string text, float x, float y, float w, float h, float min, float max, float value, Func &function) : Min(min), Max(max), Value(value) {
    X = x;
    Y = y;
    Width = w;
    Height = (h == 0) ? SLIDER_PADDING * 2 : h;
    
    Opacity = SLIDER_OPACITY;
    HandleOpacity = SLIDER_HANDLE_OPACITY;
    
    Color = SLIDER_COLOR;
    HandleColor = SLIDER_HANDLE_COLOR;
    
    Function = function;
    
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
    
    BackgroundBuffer.Create(2, Data {X, Y + Height, X, Y, X + Width, Y, X, Y + Height, X + Width, Y, X + Width, Y + Height});
    BorderBuffer.Create(2, Data {X, Y, X + Width, Y, X + Width, Y + Height, X - 0.5f, Y + Height});
    HandleBuffer.Create(2, Data {sx, Y + Height, sx, Y, sx + sw, Y, sx, Y + Height, sx + sw, Y, sx + sw, Y + Height});
    
    BorderBuffer.VertexType = GL_LINE_LOOP;
}

inline void Slider::Hover() { HandleColor = SLIDER_HANDLE_HOVER_COLOR; }
inline void Slider::Stop_Hover() { HandleColor = SLIDER_HANDLE_COLOR; }
inline void Slider::Press() { HandleColor = SLIDER_HANDLE_CLICK_COLOR; }
inline void Slider::Release() { HandleColor = SLIDER_HANDLE_COLOR; Function(); }
    
void Slider::Move(float position) {
    float w = SLIDER_WIDTH / 2;
    
    if (position <= X + w) {
        position = X + w;
    }
    else if (position >= X + Width - 1) {
        position = X + Width - 1;
    }
    
    float percentage = (position - X) / Width;
    float x = X + Width * percentage - w;
    
    HandleBuffer.Upload(Data {x, Y + Height, x, Y, x + w, Y,  x, Y + Height, x + w, Y, x + w, Y + Height});
    
    HandlePosition = x;
    Value = (Max - Min) * percentage;
}

void Slider::Draw() {
    UIElement::Draw();
    
    UIBackgroundShader->Upload(BgAlphaLoc, HandleOpacity);
    UIBackgroundShader->Upload(BgColorLoc, HandleColor);
    
    HandleBuffer.Draw();
}

Bar::Bar(std::string text, float x, float y, float w, float h, float min, float max, float value) : Min(min), Max(max), Value(value) {
    X = x;
    Y = y;
    Width = w;
    Height = (h == 0) ? BAR_PADDING * 2 : h;
    
    Opacity = BAR_BACKGROUND_OPACITY;
    Color = BAR_BACKGROUND_COLOR;
    
    BarOpacity = BAR_OPACITY;
    BarColor = BAR_COLOR;
    
    Text.Create(text, x, y);
    Text.Opacity = BAR_TEXT_OPACITY;
    Text.Color = BAR_TEXT_COLOR;
    
    BackgroundBuffer.Init(UIBackgroundShader);
    BarBuffer.Init(UIBackgroundShader);
    BorderBuffer.Init(UIBorderShader);
    
    BackgroundBuffer.Create(2, Get_Rect(X, X + Width, Y, Y + Height));
    BarBuffer.Create(2);
    BorderBuffer.Create(2, Get_Border(X, X + Width, Y, Y + Height));
    
    BorderBuffer.VertexType = GL_LINE_LOOP;
}

void Bar::Move(float value) {
    Value = value;
    
    float barEndX = X + Width * ((Value - Min) / (Max - Min));
    BarBuffer.Upload(Get_Rect(X, barEndX, Y, Y + Height));
}

void Bar::Draw() {
    UIElement::Draw();
    
    UIBackgroundShader->Upload(BgAlphaLoc, BarOpacity);
    UIBackgroundShader->Upload(BgColorLoc, BarColor);
    
    BarBuffer.Draw();
}

Image::Image(std::string file, int texID, float x, float y, float scale) : X(x), Y(y), Scale(scale), TexID(texID) {
    glActiveTexture(GL_TEXTURE0 + TexID);
    
    std::tie(Texture, Width, Height) = Load_Texture(file);
    Width *= Scale;
    Height *= Scale;
    
    glBindTexture(GL_TEXTURE_2D, Texture);
    
    ImageBuffer.Init(UITextureShader);
    ImageBuffer.Create(2, 2, Get_Tex_Rect(X, X + Width, Y, Y + Height));
}

void Image::Center() {
    X = (SCREEN_WIDTH - Width) / 2;
    ImageBuffer.Upload(Get_Tex_Rect(X, X + Width, Y, Y + Height));
}

void Image::Draw() {
    UITextureShader->Upload("tex", TexID);
    ImageBuffer.Draw();
}

Background::Background(float x, float y, float w, float h, bool border, glm::vec2 gridWidth, glm::vec2 pad) : X(x), Y(y), Width(w), Height(h) {
    Opacity = BACKGROUND_OPACITY;
    Color = BACKGROUND_COLOR;
    GridColor = BACKGROUND_BORDER_COLOR;
    
    Width = w;
    Height = h;
    
    BackgroundBuffer.Init(UIBackgroundShader);
    BackgroundBuffer.Create(2, Get_Rect(X, X + Width, Y, Y + Height));
    
    if (border) {
        GridSet = true;
        GridBuffer.Init(UIBorderShader);
        
        Data gridData;
        
        if (gridWidth != glm::vec2(0, 0)) {
            GridWidth = gridWidth;
            
            for (float gx = X + pad.x; gx <= X + Width - pad.x; gx += GridWidth.x) {
                Extend(gridData, gx, Y + pad.y, gx, Y + Height - pad.y);
            }
            
            for (float gy = Y + pad.y; gy <= Y + Height - pad.y; gy += GridWidth.y) {
                Extend(gridData, X + pad.x, gy, X + Width - pad.x, gy);
            }
        }
        else {
            Extend(gridData, Get_Border(X + 5, X + Width - 5, Y + 5, Y + Height - 5));
        }
        
        GridBuffer.Create(2, gridData);
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
            Extend(gridData, gx, Y, gx, Y + Height);
        }
        
        for (float gy = Y; gy <= Y + Height; gy += GridWidth.y) {
            Extend(gridData, X, gy, X + Width, gy);
        }
        
        Extend(gridData, X - 0.5f, Y + Height, X + 0.5f, Y + Height);
        
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
        UIBorderShader->Upload(BorderColorLoc, GridColor);
        GridBuffer.Draw();
    }
}

OrthoElement::OrthoElement(int type, float x, float y, float scale) {
    OrthoBuffer.Init(UI3DShader);
    
    Type = type;
    Scale = scale;
    
    if (type == 0) {
        OrthoBuffer.Create(3, 2, 2);
    }
    else {
        OrthoBuffer.Create(3, 2, 2, Get_3D_Mesh(type, x, y, true));
    }
}

void OrthoElement::Mesh(int type, float x, float y) {
    Type = type;
    
    if (Type != 0) {
        OrthoBuffer.Upload(Get_3D_Mesh(Type, x, y, true));
    }
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
    TextColorLoc = TextShader->Get_Location("textColor");
    
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
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            (unsigned int)face->glyph->advance.x,
        };
        
        Characters.insert(std::pair<char, Character>(c, character));
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    
    Data data;
    data.resize(4 * 6);
    
    TextBuffer.Init(TextShader);
    TextBuffer.Create(2, 2, data);
    
    TextShader->Upload("projection", glm::ortho(0.0f, (float)SCREEN_WIDTH, 0.0f, (float)SCREEN_HEIGHT));
    TextShader->Upload("text", TEXT_TEXTURE_UNIT);
}

void Interface::Mouse_Handler(float x, float y) {
    if (ActiveDocument == "") {
        return;
    }
    
    if (Holding && ActiveSlider != nullptr) {
        ActiveSlider->Move(x);
        return;
    }
    
    for (auto &button : Buttons[ActiveDocument]) {
        if (x >= button.second.X && x <= (button.second.X + button.second.Width)) {
            if (y >= button.second.Y && y <= (button.second.Y + button.second.Height)) {
                Holding ? button.second.Press() : button.second.Hover();
                
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
                Holding ? slider.second.Press() : slider.second.Hover();
                
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
        Holding = false;
        return;
    }
    
    Holding = (action == GLFW_PRESS);
    
    if (ActiveElement == BUTTON) {
        Holding ? ActiveButton->Press() : ActiveButton->Release();
    }
    
    else if (ActiveElement == SLIDER) {
        Holding ? ActiveSlider->Press() : ActiveSlider->Release();
    }
    
    if (!Holding) {
        ActiveElement = NONE;
        ActiveSlider = nullptr;
        ActiveButton = nullptr;
    }
}

void Interface::Draw_Document(std::string document) {
    glDisable(GL_DEPTH_TEST);
    
    for (auto &bg : Backgrounds[document]) { bg.second.Draw(); }
    for (auto &image : Images[document]) { image.second.Draw(); }
    for (auto &button : Buttons[document]) { button.second.Draw(); }
    for (auto &slider : Sliders[document]) { slider.second.Draw(); }
    for (auto &bar : Bars[document]) { bar.second.Draw(); }
    for (auto &object : OrthoElements[document]) { object.second.Draw(); }
    for (auto &text : TextElements[document]) { text.second.Draw(); }
    
    glEnable(GL_DEPTH_TEST);
}

float Interface::Get_String_Width(std::string string) {
    float currentWidth = 0;
    for (char const &c : string) { currentWidth += (Characters[c].Advance >> 6); }
    return currentWidth;
}

std::vector<std::string> Interface::Get_Fitting_String(std::string string, int width) {
    float currentWidth = 0;
    int index = 0;
    int prevIndex = 0;
    bool addedString = false;
    bool ignoreNext = false;
    
    std::vector<std::string> partStrings;
    
    for (char const &c : string) {
        if (c == '&' || ignoreNext) {
            ignoreNext = !ignoreNext;
        }
        
        else {
            currentWidth += (Characters[c].Advance >> 6);
            addedString = currentWidth > width;
            
            if (addedString) {
                if (c != ' ') {
                    unsigned long lastSpacePos = string.substr(prevIndex, index).rfind(' ');
                    
                    if (lastSpacePos != std::string::npos) {
                        partStrings.push_back(string.substr(prevIndex, lastSpacePos));
                        prevIndex = int(lastSpacePos) + 1;
                        currentWidth = Get_String_Width(string.substr(prevIndex, index - prevIndex));
                    }
                    
                    else {
                        partStrings.push_back(string.substr(prevIndex, index - prevIndex));
                        prevIndex = index;
                        currentWidth = 0;
                    }
                }
                
                else {
                    partStrings.push_back(string.substr(prevIndex, index - prevIndex));
                    prevIndex = index + 1;
                    currentWidth = 0;
                }
            }
        }
        
        ++index;
    }
    
    if (!addedString) {
        partStrings.push_back(string.substr(prevIndex));
    }
    
    return partStrings;
}