#include "Interface.h"

#include <algorithm>

#include <glm/gtc/matrix_transform.hpp>

#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

#include <SOIL/SOIL.h>
#include <FreeImagePlus.h>

#include <unicode/ustream.h>

#include "UI.h"
#include "main.h"
#include "Blocks.h"
#include "Shader.h"
#include "Inventory.h"

#ifdef WIN32
    #undef min
    #undef max
#endif

#ifdef __APPLE__

errno_t localtime_s(std::tm* tm, std::time* time) {
    tm = std::localtime(time);
    return tm == nullptr;
}

#endif


std::string Interface::ActiveDocument  = "";
std::string Interface::HoveringType    = "";
void*       Interface::HoveringElement = nullptr;
bool        Interface::Holding         = false;

static std::map<std::string, std::map<std::string, Bar         >> Bars;
static std::map<std::string, std::map<std::string, Slot        >> Slots;
static std::map<std::string, std::map<std::string, Image       >> Images;
static std::map<std::string, std::map<std::string, Button      >> Buttons;
static std::map<std::string, std::map<std::string, Custom      >> Customs;
static std::map<std::string, std::map<std::string, Slider      >> Sliders;
static std::map<std::string, std::map<std::string, TextBox     >> TextBoxes;
static std::map<std::string, std::map<std::string, Background  >> Backgrounds;
static std::map<std::string, std::map<std::string, TextElement >> TextElements;
static std::map<std::string, std::map<std::string, OrthoElement>> OrthoElements;

const std::string FONT      = "Roboto";
const int         FONT_SIZE = 15;

static Shader* TextShader;
static Shader* UI3DShader;
static Shader* UIBorderShader;
static Shader* UITextureShader;
static Shader* UIBackgroundShader;

static glm::ivec2 TEXT_ATLAS_SIZE = {0, 0};

static int       TEXT_BOX_HORZ_PADDING;

static float     BAR_PADDING;
static float     SLIDER_WIDTH;
static float     TEXT_PADDING;
static float     BUTTON_PADDING;
static float     SLIDER_PADDING;
static float     SLOT_BLOCK_SCALE;

static glm::vec2 SLOT_SIZE;
static glm::vec2 SLOT_PADDING;
static glm::vec2 SLOT_TEXT_PADDING;

const  int       TEXT_TEXTURE_UNIT         = 10;
const  int       TEXT_GLYPHS               = 128;

const  glm::vec3 SLOT_BG_COLOR             = glm::vec3(0.1f);
const  glm::vec3 SLOT_HOVER_COLOR          = glm::vec3(0.7f);

const  float     BUTTON_OPACITY            = 1.0f;
const  float     BUTTON_TEXT_OPACITY       = 1.0f;
const  glm::vec3 BUTTON_COLOR              = glm::vec3(0.5f);
const  glm::vec3 BUTTON_HOVER_COLOR        = glm::vec3(0.7f);
const  glm::vec3 BUTTON_CLICK_COLOR        = glm::vec3(0.3f, 0.3f, 0.8f);
const  glm::vec3 BUTTON_TEXT_COLOR         = glm::vec3(1.0f);

const  float     SLIDER_OPACITY            = 1.0f;
const  float     SLIDER_HANDLE_OPACITY     = 1.0f;
const  float     SLIDER_TEXT_OPACITY       = 1.0f;
const  glm::vec3 SLIDER_COLOR              = glm::vec3(0.5f);
const  glm::vec3 SLIDER_HANDLE_COLOR       = glm::vec3(0.7f);
const  glm::vec3 SLIDER_HANDLE_HOVER_COLOR = glm::vec3(0.9f);
const  glm::vec3 SLIDER_HANDLE_CLICK_COLOR = glm::vec3(0.3f, 0.3f, 0.8f);
const  glm::vec3 SLIDER_TEXT_COLOR         = glm::vec3(1.0f);

const  float     BAR_OPACITY               = 1.0f;
const  float     BAR_TEXT_OPACITY          = 1.0f;
const  float     BAR_BACKGROUND_OPACITY    = 1.0f;
const  glm::vec3 BAR_BACKGROUND_COLOR      = glm::vec3(0.0f, 1.0f, 0.0f);
const  glm::vec3 BAR_COLOR                 = glm::vec3(0.2f);
const  glm::vec3 BAR_TEXT_COLOR            = glm::vec3(1.0f);

const  float     BACKGROUND_OPACITY        = 0.7f;
const  glm::vec3 BACKGROUND_COLOR          = glm::vec3(0.0f);
const  glm::vec3 BACKGROUND_BORDER_COLOR   = glm::vec3(0.5f);

const std::map<char, glm::vec3> ColorCodes = {
    {'0', {0.000, 0.000, 0.000} }, // Black
    {'1', {0.000, 0.000, 0.666} }, // Dark Blue
    {'2', {0.000, 0.666, 0.000} }, // Dark Green
    {'3', {0.000, 0.666, 0.666} }, // Dark Aqua
    {'4', {0.666, 0.000, 0.000} }, // Dark Red
    {'5', {0.666, 0.000, 0.666} }, // Dark Purple
    {'6', {1.000, 0.666, 0.000} }, // Gold
    {'7', {0.666, 0.666, 0.666} }, // Gray
    {'8', {0.333, 0.333, 0.333} }, // Dark Gray
    {'9', {0.333, 0.333, 1.000} }, // Blue
    {'a', {0.333, 1.000, 0.333} }, // Green
    {'b', {0.333, 1.000, 1.000} }, // Aqua
    {'c', {1.000, 0.333, 0.333} }, // Red
    {'d', {1.000, 0.333, 1.000} }, // Light Purple
    {'e', {1.000, 1.000, 0.333} }, // Yellow
    {'f', {1.000, 1.000, 1.000} }, // White
};

struct CharacterInfo {
  glm::vec2 Advance;
  glm::vec2 BitmapSize;
  glm::vec2 BitmapOffset;

  glm::vec2 Offset = {0, 0};
};

static std::map<char, CharacterInfo> Characters;

Data Get_3D_Mesh(const Block* block, float x, float y, bool offsets) {
    Data data;

    x *= 2.005f;
    y *= 2.005f;

    if (block->HasIcon) {
        for (int i = 0; i < 6; i++) {
            for (int j = 0; j < 6; j++) {
                Extend(data, vertices[i][j]);
                Extend(data, tex_coords[i][j]);
                data.push_back(static_cast<float>(block->Icon));

                if (offsets) {
                    Extend(data, x, y);
                }
            }
        }
    }

    else if (block->HasCustomData) {
        for (auto const &element : block->CustomData) {
            for (unsigned long i = 0; i < 6; i++) {
                for (unsigned long j = 0; j < 6; j++) {
                    Extend(data, element[i][j].first);
                    Extend(data, element[i][j].second);

                    if (offsets) {
                        Extend(data, x, y);
                    }
                }
            }
        }
    }

    else {
        for (unsigned long i = 0; i < 6; i++) {
            for (unsigned long j = 0; j < 6; j++) {
                Extend(data, vertices[i][j]);

                if (block->MultiTextures) {
                    Extend(data, tex_coords[i][j]);
                    data.push_back(static_cast<float>(block->Textures[i]));
                }
                else if (block->HasTexture) {
                    Extend(data, tex_coords[i][j]);
                    data.push_back(static_cast<float>(block->Texture));
                }

                if (offsets) {
                    Extend(data, x, y);
                }
            }
        }
    }

    return data;
}

std::tuple<unsigned int, int, int> Load_Texture(std::string file, bool mipmap, float afLevel) {
    std::string path = "Images/" + file;

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmap ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, afLevel);

    int width, height;
    unsigned char* image = SOIL_load_image(path.c_str(), &width, &height, 0, SOIL_LOAD_RGBA);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glBindTexture(GL_TEXTURE_2D, 0);
    SOIL_free_image_data(image);

    return std::make_tuple(texture, width, height);
}

unsigned int Load_Array_Texture(std::string file, glm::ivec2 subCount, int mipmap, float afLevel) {
    std::string path = "Images/" + file;

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);

    FREE_IMAGE_FORMAT format = FreeImage_GetFileType(path.c_str(), 0);
    FIBITMAP* image = FreeImage_Load(format, path.c_str());

    FreeImage_FlipVertical(image);

    int width = static_cast<int>(FreeImage_GetWidth(image));
    int height = static_cast<int>(FreeImage_GetHeight(image));

    glm::ivec2 subSize(width / subCount.x, height / subCount.y);

    glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY_EXT, afLevel);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S,     GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T,     GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    if (mipmap > 0) {
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    }
    else {
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }

    glTexStorage3D(
        GL_TEXTURE_2D_ARRAY, mipmap + 1, GL_RGBA8,
        subSize.x, subSize.y, (width * height) / (subSize.x * subSize.y)
    );

    int layer = 0;

    for (int h = 0; h < height; h += subSize.y) {
        for (int w = 0; w < width; w += subSize.x) {
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0,
                layer++, subSize.x, subSize.y, 1, GL_BGRA, GL_UNSIGNED_BYTE,
                reinterpret_cast<void*>(FreeImage_GetBits(
                    FreeImage_Copy(image, w, height - h, w + subSize.x, height - h - subSize.y)
                ))
            );
        }
    }

    if (mipmap > 0) {
        glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    }

    FreeImage_Unload(image);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    return texture;
}

void Take_Screenshot() {
    std::tm tm;
    std::time_t t = std::time(nullptr);
    localtime_s(&tm, &t);
    std::stringstream ss;

    ss << std::put_time(&tm, "%F %H-%M-%S");
    std::string fileName = "Screenshots/" + ss.str() + ".bmp";
    SOIL_save_screenshot(fileName.c_str(), SOIL_SAVE_TYPE_BMP, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
}

Custom::Custom(std::string name, float x, float y, Data &data) {
    Name = name;
    X = x;
    Y = y;
        
    ModelMatrix = glm::translate(ModelMatrix, glm::vec3(X, Y, 0));
    
    Storage.Init(UIBackgroundShader);
    Storage.Create(2, data);
}

void Custom::Draw() {
    UIBackgroundShader->Upload("color", Color);
    UIBackgroundShader->Upload("model", ModelMatrix);
    Storage.Draw();
    UIBackgroundShader->Upload("model", glm::mat4());
}

void TextElement::Create(std::string name, std::string text, float x, float y, float opacity, glm::vec3 color, float scale) {
	x = std::floor(x);
	y = std::floor(y);

    OriginalX = x;
    OriginalY = y;

    X = x;
    Y = y;
    Name = name;
    Text = text;
    Color = color;
    Scale = scale;
    Opacity = opacity;
    Width = Get_Width();

    TextBuffer.Init(TextShader);
    TextBuffer.Create(2, 2, 3);

    Mesh();
}

void TextElement::Center(float x, float y, float width, glm::bvec2 axes) {
    Centered = axes;
    CenterWidth = width;

    if (axes.x) {
        X = std::floor(x + (width - Width) / 2);
    }

    if (axes.y) {
        Y = std::floor(y + TEXT_PADDING - float(FONT_SIZE / 6));
    }
}

void TextElement::Set_Text(std::string newText) {
    Text = newText;
    Width = Get_Width();

    Center(OriginalX, OriginalY, CenterWidth, Centered);
    Mesh();
}

void TextElement::Mesh() {
    Data data;

    bool skipNext = false;
    glm::vec2 charPos = {0, 0};
    glm::vec3 textColor = Color;

    for (char const &c : Text) {
        if (c == '&' || skipNext) {
            if (skipNext) {
                try {
                    textColor = ColorCodes.at(c);
                }
                catch (const std::out_of_range) {
                    throw std::runtime_error("Error! Invalid color code in string \"" + Text + "\".");
                }
            }

            skipNext = (c == '&');
            continue;
        }

        CharacterInfo ch = Characters[c];

        glm::vec2 size = ch.BitmapSize * Scale;
        glm::vec2 p1 = charPos + ch.BitmapOffset * Scale - glm::vec2(0, size.y);
        glm::vec2 p2 = p1 + size;

        charPos += ch.Advance * Scale;

        if (size.x == 0 || size.y == 0) {
            continue;
        }

        glm::vec2 t1 = ch.Offset;
        glm::vec2 t2 = t1 + ch.BitmapSize / static_cast<glm::vec2>(TEXT_ATLAS_SIZE);

        Extend(data,
            p1.x, p1.y, t1.x, t2.y, EXPAND_VEC3(textColor),
            p2.x, p1.y, t2.x, t2.y, EXPAND_VEC3(textColor),
            p2.x, p2.y, t2.x, t1.y, EXPAND_VEC3(textColor),
            p1.x, p1.y, t1.x, t2.y, EXPAND_VEC3(textColor),
            p2.x, p2.y, t2.x, t1.y, EXPAND_VEC3(textColor),
            p1.x, p2.y, t1.x, t1.y, EXPAND_VEC3(textColor)
        );
    }

    TextBuffer.Upload(data);
}

void TextElement::Draw() {
    if (Opacity == 0.0f || Text == "") {
        return;
    }

    TextShader->Upload("Position", glm::vec2(X, Y));
    TextShader->Upload("Opacity", Opacity);
    TextBuffer.Draw();
}

float TextElement::Get_Width() {
    float width = 0;
    bool skipNext = false;

    for (char const &c : Text) {
        if (c == '&' || skipNext) {
            skipNext = (c == '&');
            continue;
        }

        width += Characters[c].Advance.x * Scale;
    }

    return width;
}

void UIElement::Draw() {
    UIBackgroundShader->Upload("color", Color);
    UIBackgroundShader->Upload("alpha", Opacity);

    BackgroundBuffer.Draw();

    UIBorderShader->Upload("color", glm::vec3(0));
    BorderBuffer.Draw();

    Text.Draw();
}

Button::Button(std::string name, std::string text, float x, float y, float w, float h, Func &function) {
    Height = (Height != 0) ? h : BUTTON_PADDING * 2;

    X = x;
    Y = y;
    Width = w;
    Name = name;

    Opacity = BUTTON_OPACITY;
    Color = BUTTON_COLOR;

    Function = function;

    Text.Create(name, text, X, Y);
    Text.Center(X, Y, Width);

    Text.Opacity = BUTTON_TEXT_OPACITY;
    Text.Color = BUTTON_TEXT_COLOR;

    BackgroundBuffer.Init(UIBackgroundShader);
    BorderBuffer.Init(UIBorderShader);

    BackgroundBuffer.Create(2, Data {
        X, Y + Height, X, Y, X + Width, Y, X, Y + Height, X + Width, Y, X + Width, Y + Height
    });
    BorderBuffer.Create(2, Data {
        X, Y, X + Width, Y, X + Width, Y + Height, X, Y + Height
    });

    BorderBuffer.VertexType = GL_LINE_LOOP;
}

inline void Button::Hover() { Color = BUTTON_HOVER_COLOR; }
inline void Button::Stop_Hover() { Color = BUTTON_COLOR; }
inline void Button::Press() { Color = BUTTON_CLICK_COLOR; }
inline void Button::Release() {
    Color = BUTTON_COLOR;
    Function(this);

    Interface::HoveringType = "";
    Interface::HoveringElement = nullptr;
}

Slider::Slider(std::string name, std::string text, float x, float y, float w, float h, float min,
    float max, float value, Func &function) : Value(value), Min(min), Max(max) {
    
    std::tie(X, Y, Width, Name) = {x, y, w, name};
    
    Height = (h == 0) ? SLIDER_PADDING * 2 : h;
    
    std::tie(Opacity, HandleOpacity, Color, HandleColor) = std::make_tuple(
        SLIDER_OPACITY, SLIDER_HANDLE_OPACITY,
        SLIDER_COLOR, SLIDER_HANDLE_COLOR
    );

    Function = function;

    Text.Create(name, text, X, Y);
    Text.Center(X, Y, Width);

    Text.Opacity = SLIDER_TEXT_OPACITY;
    Text.Color = SLIDER_TEXT_COLOR;
    
    float sw = SLIDER_WIDTH / 2;
    float sx = x + w * ((Value - Min) / (Max - Min));
    HandlePosition = sx;

    BackgroundBuffer.Init(UIBackgroundShader);
    HandleBuffer.Init(UIBackgroundShader);
    BorderBuffer.Init(UIBorderShader);

    BackgroundBuffer.Create(2, Data {
        X, Y + Height, X, Y, X + Width, Y, X, Y + Height, X + Width, Y, X + Width, Y + Height
    });
    BorderBuffer.Create(2, Data {
        X, Y, X + Width, Y, X + Width, Y + Height, X - 0.5f, Y + Height
    });
    HandleBuffer.Create(2, Data {
        sx, Y + Height, sx, Y, sx + sw, Y, sx, Y + Height, sx + sw, Y, sx + sw, Y + Height
    });

    BorderBuffer.VertexType = GL_LINE_LOOP;
}

inline void Slider::Hover() { HandleColor = SLIDER_HANDLE_HOVER_COLOR; }
inline void Slider::Stop_Hover() { HandleColor = SLIDER_HANDLE_COLOR; }
inline void Slider::Press() { HandleColor = SLIDER_HANDLE_CLICK_COLOR; }

void Slider::Release() {
    HandleColor = SLIDER_HANDLE_COLOR;
    Move(std::ceil(Value), true);
    Function(this);
}

void Slider::Move(float position, bool setValue) {
    float w = SLIDER_WIDTH / 2;
    float x;

    if (setValue) {
        x = X + Width * ((position - 0.5f) / (Max - Min)) - w;
    }
    else {
        if (position <= X + w) {
            position = X + w;
        }
        else if (position >= X + Width - 1) {
            position = X + Width - 1;
        }

        float percentage = (position - X) / Width;
        x = X + Width * percentage - w;

        int oldValue = static_cast<int>(std::round(Value));
        Value = (Max - Min) * percentage;

        Text.Text.replace(
            Text.Text.find(std::to_string(oldValue)),
            std::to_string(oldValue).length(),
            std::to_string(static_cast<int>(std::round(Value)))
        );
        Text.Mesh();
    }

    HandleBuffer.Upload(Data {x, Y + Height, x, Y, x + w, Y,  x, Y + Height, x + w, Y, x + w, Y + Height});
    HandlePosition = x;
}

void Slider::Draw() {
    UIElement::Draw();

    UIBackgroundShader->Upload("color", HandleColor);
    UIBackgroundShader->Upload("alpha", HandleOpacity);

    HandleBuffer.Draw();
}

Bar::Bar(std::string name, std::string text, float x, float y, float w, float h, float min, float max, float value)
    : Value(value), Min(min), Max(max) {
    X = x;
    Y = y;
    Width = w;
    Name = name;
    Height = (h == 0) ? BAR_PADDING * 2 : h;

    Opacity = BAR_BACKGROUND_OPACITY;
    Color = BAR_BACKGROUND_COLOR;

    BarOpacity = BAR_OPACITY;
    BarColor = BAR_COLOR;

    Text.Create(name, text, x, y);
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

    UIBackgroundShader->Upload("color", BarColor);
    UIBackgroundShader->Upload("alpha", BarOpacity);

    BarBuffer.Draw();
}

Image::Image(std::string name, std::string file, int texID, float x, float y, float scale) : Scale(scale), TexID(texID) {
    X = x;
    Y = y;
    Name = name;

    glActiveTexture(GL_TEXTURE0 + static_cast<unsigned int>(TexID));

	auto imageData = Load_Texture(file);
	Texture = std::get<0>(imageData);
	Width = static_cast<float>(std::get<1>(imageData)) * Scale;
	Height = static_cast<float>(std::get<2>(imageData)) * Scale;

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

Background::Background(std::string name, float x, float y, float w, float h, bool border, glm::vec2 gridWidth, glm::vec2 pad) {
    X = x;
    Y = y;
    Name = name;
    Width = w;
    Height = h;

    Opacity = BACKGROUND_OPACITY;
    Color = BACKGROUND_COLOR;
    GridColor = BACKGROUND_BORDER_COLOR;

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

            // Fix for missing pixel in upper-left corner.
            Extend(gridData, X + pad.x - 0.5f, Y + Height - pad.y, X + pad.x, Y + Height - pad.y);
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
    UIBackgroundShader->Upload("color", Color);
    UIBackgroundShader->Upload("alpha", Opacity);
    BackgroundBuffer.Draw();

    if (GridSet) {
        UIBorderShader->Upload("color", GridColor);
        GridBuffer.Draw();
    }
}

OrthoElement::OrthoElement(std::string name, int type, int data, float x, float y, float scale) {
    OrthoBuffer.Init(UI3DShader);

    X = x;
    Y = y;
    Name = name;
    Type = type;
    Scale = scale;

    if (type == 0) {
        OrthoBuffer.Create(3, 3, 2);
    }
    else {
        OrthoBuffer.Create(3, 3, 2, Get_3D_Mesh(Blocks::Get_Block(Type, data), X, Y, true));
    }
}

void OrthoElement::Mesh(int type, int data, float x, float y) {
    if (x != 0) {
        X = x;
    }

    if (y != 0) {
        Y = y;
    }

    Type = type;

    if (Type == 0) {
        return;
    }

    const Block* block = Blocks::Get_Block(type, data);
    ModelMatrix = glm::mat4();

    if (!block->HasIcon) {
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(20.0f), glm::vec3(1, 0, 0));
        ModelMatrix = glm::rotate(ModelMatrix, 45.0f, glm::vec3(0, 1, 0));
    }

    OrthoBuffer.Upload(Get_3D_Mesh(block, X, Y, true));
}

void OrthoElement::Draw() {
    if (Type == 0) {
        return;
    }

    UI3DShader->Upload("model", ModelMatrix);
    UI3DShader->Upload("scale", Scale);
    OrthoBuffer.Draw();
}

TextBox::TextBox(std::string name, float x, float y, float w, float h) {
    X = x;
    Y = y;
    Width = w;
    Height = h;
    Name = name;

    BG = Background(name, x, y, w, h, true);

    MaxWidth = Width - TEXT_BOX_HORZ_PADDING * 2;

    int textPad = static_cast<int>((h - FONT_SIZE) / 2);

    Cursor.Create(name, "|", x + TEXT_BOX_HORZ_PADDING, y + textPad);
    TextEl.Create(name, Text, x + TEXT_BOX_HORZ_PADDING, y + textPad);

    Cursor.Opacity = 0;
}

void TextBox::Key_Handler(int key) {
    if (key == GLFW_KEY_BACKSPACE && CursorPos > 0) {
        --CursorPos;
        Text.erase(CursorPos, 1);
        Update();
    }

    else if (key == GLFW_KEY_LEFT && CursorPos > 0) {
        --CursorPos;
        Update();
    }

    else if (key == GLFW_KEY_RIGHT && CursorPos < Text.length()) {
        ++CursorPos;
        Update();
    }
}

void TextBox::Input(unsigned int codepoint) {
    UnicodeString string(static_cast<UChar32>(codepoint));
    std::string str;
    string.toUTF8String(str);

    Text.insert(CursorPos, str);

    ++CursorPos;
    Update();
}

void TextBox::Set_Cursor_Visibility(bool cursorVisible) {
    Cursor.Opacity = cursorVisible;
}

void TextBox::Update() {
    TextWidth = Interface::Get_String_Width(Text);

    if (TextWidth > MaxWidth) {
        --CursorPos;
        Text.erase(CursorPos, 1);
        return;
    }

    Cursor.X = X + TEXT_BOX_HORZ_PADDING + Interface::Get_String_Width(Text.substr(0, CursorPos));

    TextEl.Set_Text(Text);
    TextEl.Mesh();
}

void TextBox::Clear() {
    Text = "";
    TextWidth = 0;
    CursorPos = 0;
    Cursor.X = X + TEXT_BOX_HORZ_PADDING;
    TextEl.Set_Text("");
    TextEl.Mesh();
}

void TextBox::Draw() {
    if (Visible) {
        BG.Draw();
        Cursor.Draw();
        TextEl.Draw();
    }
}

Slot::Slot(std::string name, float x, float y, float scale, Stack contents) : SlotSize(scale), Contents(contents) {
    X = x;
    Y = y;
    Name = name;
    Width = Scale_X(SlotSize);
    Height = Scale_Y(SlotSize);

    glm::vec2 textPos  = glm::vec2(X, Y) + SLOT_TEXT_PADDING * (SlotSize / 80);
    glm::vec2 modelPos = glm::vec2(X, Y) + SLOT_PADDING      * (SlotSize / 80);

    BG = Background(Name, X, Y, Width, Height, true, Scale(scale));
    BG.Color = SLOT_BG_COLOR;
    ItemCount.Create(Name, std::to_string(Contents.Size), textPos.x, textPos.y, static_cast<float>(Contents.Size > 0));
    ItemModel = OrthoElement(Name, Contents.Type, Contents.Data, modelPos.x, modelPos.y, Width);

    Mesh();
}

void Slot::Set_Contents(const Stack &stack) {
    Contents = stack;
    Mesh();
}

void Slot::Hover() {
    Hovering = true;
    BG.Color = SLOT_HOVER_COLOR;
}

void Slot::Stop_Hover() {
    Hovering = false;
    BG.Color = SLOT_BG_COLOR;
}

void Slot::Mesh() {
    ItemCount.Opacity = (Contents.Size > 0 && Contents.Type > 0);
    ItemCount.Set_Text(std::to_string(Contents.Size));
    ItemModel.Mesh(Contents.Type, Contents.Data, glm::vec2(X, Y) + SLOT_PADDING * (SlotSize / 80));

    if (SyncedSlot != nullptr) {
        SyncedSlot->Set_Contents(Contents);
    }
}

void Slot::Draw() {
    if (Hovering) {
        ItemModel.Draw();
        ItemCount.Draw();
        BG.Draw();
    }
    else {
        BG.Draw();
        ItemModel.Draw();
        ItemCount.Draw();
    }
}

void Interface::Init() {
    Init_UI_Scale();
    Init_Shaders();
    Init_Text();
}

void Interface::Init_UI_Scale() {
    TEXT_BOX_HORZ_PADDING = static_cast<int>(Scale_X(10));
    SLOT_BLOCK_SCALE      = std::min(Scale_X(80), Scale_Y(80));

    SLOT_PADDING          = Scale(10, 20);
    SLOT_TEXT_PADDING     = Scale(5, 10);

    SLIDER_WIDTH          = Scale_X(10);

    BUTTON_PADDING        = Scale_Y(20);
    SLIDER_PADDING        = Scale_Y(20);
    TEXT_PADDING          = Scale_Y(20);
    BAR_PADDING           = Scale_Y(20);

    SLOT_SIZE             = glm::vec2(SLOT_BLOCK_SCALE);
}

void Interface::Init_Shaders() {
    glActiveTexture(GL_TEXTURE0);

    UIBackgroundShader = new Shader("ui");
    UIBorderShader = new Shader("uiBorder");
    UITextureShader = new Shader("uiTex");
    UI3DShader = new Shader("ortho");

    glm::mat4 model;
    model = glm::rotate(model, glm::radians(20.0f), glm::vec3(1, 0, 0));
    model = glm::rotate(model, 45.0f, glm::vec3(0, 1, 0));

    glm::mat4 projection = glm::ortho(
        0.0f, static_cast<float>(SCREEN_WIDTH), 0.0f, static_cast<float>(SCREEN_HEIGHT)
    );

    UIBackgroundShader->Upload("projection", projection);
    UIBackgroundShader->Upload("model", glm::mat4());

    UIBorderShader->Upload("projection", projection);
    UIBorderShader->Upload("color", glm::vec3(0));

    UITextureShader->Upload("projection", projection);
    UITextureShader->Upload("tex", 0);

    UI3DShader->Upload("tex", 0);
    UI3DShader->Upload("model", model);
    UI3DShader->Upload("projection", glm::ortho(
        0.0f, static_cast<float>(SCREEN_WIDTH), 0.0f,
        static_cast<float>(SCREEN_HEIGHT), -1000.0f, 1000.0f
    ));
}

void Interface::Init_Text() {
    TextShader = new Shader("text");

    FT_Library ft;
    FT_Face face;

    FT_Init_FreeType(&ft);
    FT_New_Face(ft, std::string("Fonts/" + FONT + ".ttf").c_str(), 0, &face);
    FT_Set_Pixel_Sizes(face, 0, FONT_SIZE);

    FT_GlyphSlot g = face->glyph;

    for (unsigned char c = 32; c < TEXT_GLYPHS; c++) {
        FT_Load_Char(face, c, FT_LOAD_RENDER);
        TEXT_ATLAS_SIZE.x += g->bitmap.width;
        TEXT_ATLAS_SIZE.y = std::max(TEXT_ATLAS_SIZE.y, static_cast<int>(g->bitmap.rows));
    }

    unsigned int textAtlas;
    glActiveTexture(GL_TEXTURE0 + TEXT_TEXTURE_UNIT);
    glGenTextures(1, &textAtlas);
    glBindTexture(GL_TEXTURE_2D, textAtlas);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RED,
        TEXT_ATLAS_SIZE.x, TEXT_ATLAS_SIZE.y,
        0, GL_RED, GL_UNSIGNED_BYTE, NULL
    );

    int xOffset = 0;

    for (unsigned char c = 32; c < TEXT_GLYPHS; ++c) {
        CharacterInfo ch;

        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            continue;
        }

        glTexSubImage2D(
            GL_TEXTURE_2D, 0, xOffset, 0,
            static_cast<int>(g->bitmap.width), static_cast<int>(g->bitmap.rows),
            GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer
        );

        ch.Advance = glm::vec2(g->advance.x >> 6, g->advance.y >> 6);
        ch.BitmapSize = glm::vec2(g->bitmap.width, g->bitmap.rows);
        ch.BitmapOffset = glm::vec2(g->bitmap_left, g->bitmap_top);
        ch.Offset = glm::vec2(static_cast<float>(xOffset) / TEXT_ATLAS_SIZE.x, 0);

        Characters[static_cast<char>(c)] = ch;
        xOffset += g->bitmap.width;
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    TextShader->Upload("text", TEXT_TEXTURE_UNIT);
    TextShader->Upload("projection", glm::ortho(
        0.0f, static_cast<float>(SCREEN_WIDTH),
        0.0f, static_cast<float>(SCREEN_HEIGHT)
    ));
}

void Interface::Mouse_Handler(int x, int y) {
    if (ActiveDocument == "") {
        return;
    }

    if (Holding && HoveringType == "slider") {
        static_cast<Slider*>(HoveringElement)->Move(static_cast<float>(x));
        return;
    }

    if (HoveringType == "slot")         { static_cast<Slot*>   (HoveringElement)->Stop_Hover(); }
    else if (HoveringType == "button")  { static_cast<Button*> (HoveringElement)->Stop_Hover(); }
    else if (HoveringType == "slider")  { static_cast<Slider*> (HoveringElement)->Stop_Hover(); }
    else if (HoveringType == "textBox") { static_cast<TextBox*>(HoveringElement)->Set_Cursor_Visibility(false); }

    for (auto &slot : Slots[ActiveDocument]) {
        if (In_Range(x, glm::vec2(slot.second.X, slot.second.Width))) {
            if (In_Range(y, glm::vec2(slot.second.Y, slot.second.Height))) {
                slot.second.Hover();

                if (Holding) {
                    Inventory::Dragging_Slot(&slot.second);
                }

                HoveringType = "slot";
                HoveringElement = &slot.second;
                return;
            }
        }
    }
    
    if (ActiveDocument == UI::CustomDocument) {
        for (auto &slot : Slots["inventory"]) {
            if (In_Range(x, glm::vec2(slot.second.X, slot.second.Width))) {
                if (In_Range(y, glm::vec2(slot.second.Y, slot.second.Height))) {
                    slot.second.Hover();

                    if (Holding) {
                        Inventory::Dragging_Slot(&slot.second);
                    }

                    HoveringType = "slot";
                    HoveringElement = &slot.second;
                    return;
                }
            }
        }
    }

    for (auto &button : Buttons[ActiveDocument]) {
        if (In_Range(x, glm::vec2(button.second.X, button.second.Width))) {
            if (In_Range(y, glm::vec2(button.second.Y, button.second.Height))) {
                Holding ? button.second.Press() : button.second.Hover();

                HoveringType = "button";
                HoveringElement = &button.second;
                return;
            }
        }
    }

    for (auto &slider : Sliders[ActiveDocument]) {
        if (In_Range(x, glm::vec2(slider.second.HandlePosition, SLIDER_WIDTH))) {
            if (In_Range(y, glm::vec2(slider.second.Y, slider.second.Height))) {
                Holding ? slider.second.Press() : slider.second.Hover();

                HoveringType = "slider";
                HoveringElement = &slider.second;
                return;
            }
        }
    }

    for (auto &box : TextBoxes[ActiveDocument]) {
        if (In_Range(x, glm::vec2(box.second.X, box.second.Width))) {
            if (In_Range(y, glm::vec2(box.second.Y, box.second.Height))) {
                HoveringType = "textBox";
                HoveringElement = &box.second;
                box.second.Set_Cursor_Visibility(true);
                return;
            }
        }
    }

    HoveringType = "";
    HoveringElement = nullptr;
}

void Interface::Click(int mouseButton, int action) {
    if (HoveringType == "") {
        Holding = false;
        return;
    }

    Holding = (action == GLFW_PRESS);

    if (HoveringType == "slot") {
        Slot* slot = static_cast<Slot*>(HoveringElement);

        if (Holding) {
            Inventory::Press_Slot(slot, mouseButton);
        }
        else {
            Inventory::Release_Slot();
        }
    }

    else if (HoveringType == "button") {
        Button* button = static_cast<Button*>(HoveringElement);
        Holding ? button->Press() : button->Release();
    }

    else if (HoveringType == "slider") {
        Slider* slider = static_cast<Slider*>(HoveringElement);
        Holding ? slider->Press() : slider->Release();
    }
    
    Inventory::Mouse_Handler(UI::MouseX, UI::MouseY);
}

void Interface::Draw_Document(std::string document) {
    glDisable(GL_DEPTH_TEST);

    for (auto &bg     : Backgrounds  [document]) { bg    .second.Draw(); }
    for (auto &image  : Images       [document]) { image .second.Draw(); }
    for (auto &button : Buttons      [document]) { button.second.Draw(); }
    for (auto &slider : Sliders      [document]) { slider.second.Draw(); }
    for (auto &bar    : Bars         [document]) { bar   .second.Draw(); }
    for (auto &slot   : Slots        [document]) { slot  .second.Draw(); }
    for (auto &object : OrthoElements[document]) { object.second.Draw(); }
    for (auto &box    : TextBoxes    [document]) { box   .second.Draw(); }
    for (auto &text   : TextElements [document]) { text  .second.Draw(); }
    for (auto &custom : Customs      [document]) { custom.second.Draw(); }

    glEnable(GL_DEPTH_TEST);
}

float Interface::Get_String_Width(std::string string) {
    float currentWidth = 0;
    for (char const &c : string) { currentWidth += Characters[c].Advance.x; }
    return currentWidth;
}

std::vector<std::string> Interface::Get_Fitting_String(std::string string, int width) {
    float currentWidth = 0;
    unsigned long index = 0;
    size_t prevIndex = 0;
    bool addedString = false;
    bool ignoreNext = false;

    std::vector<std::string> partStrings;

    for (char const &c : string) {
        if (c == '&' || ignoreNext) {
            ignoreNext = !ignoreNext;
        }

        else {
            currentWidth += Characters[c].Advance.x;
            addedString = currentWidth > width;

            if (addedString) {
                if (c != ' ') {
                    size_t lastSpacePos = string.substr(prevIndex, index).rfind(' ');

                    if (lastSpacePos != std::string::npos) {
                        partStrings.push_back(string.substr(prevIndex, lastSpacePos));
                        prevIndex = lastSpacePos + 1;
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

namespace Interface {
    void Set_Document(std::string document) {
        ActiveDocument = document;
    }

    void Add_Text(std::string name, std::string text, float x, float y) {
        TextElements[ActiveDocument].emplace(name, TextElement(name, text, std::floor(x), std::floor(y)));
    }
    void Add_Slot(std::string name, float x, float y, float scale, Stack contents) {
        Slots[ActiveDocument].emplace(name, Slot(name, x, y, scale, contents));
    }
    void Add_Text_Box(std::string name, float x, float y, float w, float h) {
        TextBoxes[ActiveDocument].emplace(name, TextBox(name, x, y, w, h));
    }
    void Add_Button(std::string name, std::string text, float x, float y, float w, float h, Func &function) {
        Buttons[ActiveDocument].emplace(name, Button(name, text, x, y, w, h, function));
    }
    void Add_Custom(std::string name, float x, float y, Data &data) {
        Customs[ActiveDocument].emplace(name, Custom(name, x, y, data));
    }
    void Add_Slider(std::string name, std::string text, float x, float y, float w, float h, float min, float max, float value, Func &function) {
        Sliders[ActiveDocument].emplace(name, Slider(name, text, x, y, w, h, min, max, value, function));
    }
    void Add_Bar(std::string name, std::string text, float x, float y, float w, float h, float min, float max, float value) {
        Bars[ActiveDocument].emplace(name, Bar(name, text, x, y, w, h, min, max, value));
    }
    void Add_Image(std::string name, std::string path, int texID, float x, float y, float scale) {
        Images[ActiveDocument].emplace(name, Image(name, path, texID, x, y, scale));
    }
    void Add_Background(std::string name, glm::vec4 dims, bool border, glm::vec2 gridWidth, glm::vec2 pad) {
        Backgrounds[ActiveDocument].emplace(name, Background(name, dims.x, dims.y, dims.z, dims.w, border, gridWidth, pad));
    }
    void Add_3D_Element(std::string name, int type, int data, float x, float y, float scale) {
        OrthoElements[ActiveDocument].emplace(name, OrthoElement(name, type, data, x, y, scale));
    }

    void Delete_Bar       (std::string name) { Bars         [ActiveDocument].erase(name); }
    void Delete_Text      (std::string name) { TextElements [ActiveDocument].erase(name); }
    void Delete_Slot      (std::string name) { Slots        [ActiveDocument].erase(name); }
    void Delete_Image     (std::string name) { Images       [ActiveDocument].erase(name); }
    void Delete_Button    (std::string name) { Buttons      [ActiveDocument].erase(name); }
    void Delete_Custom    (std::string name) { Customs      [ActiveDocument].erase(name); }
    void Delete_Slider    (std::string name) { Sliders      [ActiveDocument].erase(name); }
    void Delete_Text_Box  (std::string name) { TextBoxes    [ActiveDocument].erase(name); }
    void Delete_Background(std::string name) { Backgrounds  [ActiveDocument].erase(name); }
    void Delete_3D_Element(std::string name) { OrthoElements[ActiveDocument].erase(name); }

    Slot*         Get_Slot        (std::string name) { return &Slots        [ActiveDocument][name]; }
    Image*        Get_Image       (std::string name) { return &Images       [ActiveDocument][name]; }
    Button*       Get_Button      (std::string name) { return &Buttons      [ActiveDocument][name]; }
    Custom*       Get_Custom      (std::string name) { return &Customs      [ActiveDocument][name]; }
    Slider*       Get_Slider      (std::string name) { return &Sliders      [ActiveDocument][name]; }
    TextBox*      Get_Text_Box    (std::string name) { return &TextBoxes    [ActiveDocument][name]; }
    Background*   Get_Background  (std::string name) { return &Backgrounds  [ActiveDocument][name]; }
    OrthoElement* Get_3D_Element  (std::string name) { return &OrthoElements[ActiveDocument][name]; }
    TextElement*  Get_Text_Element(std::string name) { return &TextElements [ActiveDocument][name]; }
};