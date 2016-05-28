#pragma once

#include <string>

#include "Buffer.h"

typedef void (Func)(void);
typedef std::vector<float> Data;

extern int SCREEN_WIDTH;
extern int SCREEN_HEIGHT;

extern glm::vec2 IMAGE_SIZE;
extern glm::vec3 vertices[6][6];
extern glm::vec2 tex_coords[6][6];

extern std::map<unsigned int, glm::vec2> textureCoords;
extern std::map<unsigned int, std::vector<glm::vec2>> MultiTextures;
extern std::map<unsigned int, std::vector<std::vector<glm::vec2>>> CustomTexCoords;
extern std::map<unsigned int, std::vector<std::vector<glm::vec3>>> CustomVertices;

inline std::string Format_Vector(glm::vec3 vector) {
    return std::string("X: " + std::to_string(int(vector.x)) + "\t\tY: " + std::to_string(int(vector.y)) + "\t\tZ: " + std::to_string(int(vector.z)));
}

inline Data Get_Rect(float x1, float x2, float y1, float y2) { return Data {x1, y1, x2, y1, x2, y2, x1, y1, x2, y2, x1, y2}; }
inline Data Get_Border(float x1, float x2, float y1, float y2) { return Data { x1, y1, x2, y1, x2, y1, x2, y2, x2, y2, x1, y2, x1, y2, x1, y1 }; }
inline Data Get_Tex_Rect(float x1, float x2, float y1, float y2) { return Data { x1, y1, 0, 1, x2, y1, 1, 1, x2, y2, 1, 0, x1, y1, 0, 1, x2, y2, 1, 0, x1, y2, 0, 0}; }

Data Get_3D_Mesh(unsigned int type, float x, float y, bool offsets = false);
std::tuple<unsigned int, int, int> Load_Texture(std::string file);
void Take_Screenshot();

template <typename T> inline float Scale_X(const T x) { return (x / 1440.0f) * SCREEN_WIDTH; }
template <typename T> inline float Scale_Y(const T y) { return (y / 900.0f) * SCREEN_HEIGHT; }
template <typename T> inline glm::vec2 Scale(const T t) { return glm::vec2(Scale_X(t), Scale_Y(t)); }
template <typename X, typename Y> inline glm::vec2 Scale(const X x, const Y y) { return glm::vec2(Scale_X(x), Scale_Y(y)); }

template <typename V, typename T> inline void Extend(std::vector<V> &storage, T t) { storage.push_back(V(t)); }
template <typename V, typename T, typename... Args> inline void Extend(std::vector<V> &storage, T t, Args... args) { storage.push_back(V(t)); Extend(storage, args...); }
template <typename T> inline void Extend(std::vector<T> &storage, std::vector<T> input) { for (T const &element : input) { storage.push_back(element); } }
template <typename T> inline void Extend(std::vector<T> &storage, glm::vec2 input) { Extend(storage, T(input.x), T(input.y)); }
template <typename T> inline void Extend(std::vector<T> &storage, glm::vec3 input) { Extend(storage, T(input.x), T(input.y), T(input.z)); }
template <typename T> inline void Extend(std::vector<T> &storage, glm::vec4 input) { Extend(storage, T(input.x), T(input.y), T(input.z), T(input.w)); }

class TextElement {
public:
    std::string Text = "";
    
    float X;
    float Y;
    float Scale;
    float Opacity;
    
    float Height;
    float Width;
    
    glm::vec3 Color;
    
    TextElement() {};
    TextElement(std::string text, float x, float y, float opacity = 1.0f, glm::vec3 color = glm::vec3(1.0f), float scale = 1.0f) {
        Create(text, x, y, opacity, color, scale);
    }
    
    void Create(std::string text, float x, float y, float opacity = 1.0f, glm::vec3 color = glm::vec3(1.0f), float scale = 1.0f);
    void Center(float x, float y, float width);
    void Draw();
    
private:
    float Get_Width();
};

class UIElement {
public:
    float X;
    float Y;
    float Opacity;
    
    float Width;
    float Height;
    
    glm::vec3 Color;
    
    TextElement Text;
    
    Buffer BackgroundBuffer;
    Buffer BorderBuffer;
    
    Func* Function;
    
    void Draw();
};

class Button : public UIElement {
public:
    Button() {};
    Button(std::string text, float x, float y, float w, float h, Func &function);
    
    inline void Hover();
    inline void Stop_Hover();
    
    inline void Press();
    inline void Release();
};

class Slider : public UIElement {
public:
    float HandlePosition;
    float Value;
    
    Slider() {};
    Slider(std::string text, float x, float y, float w, float h, float min, float max, float value, Func &function);
    
    inline void Hover();
    inline void Stop_Hover();
    
    inline void Press();
    inline void Release();
    
    void Move(float position);
    void Draw();
    
private:
    float Min;
    float Max;
    
    float HandleOpacity;
    glm::vec3 HandleColor;
    
    Buffer HandleBuffer;
};

class Bar : public UIElement {
public:
    float BarOpacity;
    glm::vec3 BarColor;
    
    float Value;
    
    Bar() {};
    Bar(std::string text, float x, float y, float w, float h, float min, float max, float value);
    
    void Move(float value);
    void Draw();
    
private:
    float Min;
    float Max;
    
    Buffer BarBuffer;
};

class Image {
public:
    Image() {};
    Image(std::string file, int texID, float x, float y, float scale);
    
    void Center();
    void Draw();
    
private:
    float X;
    float Y;
    float Width;
    float Height;
    float Scale;
    
    unsigned int Texture;
    int TexID;
    
    Buffer ImageBuffer;
};

class Background {
public:
    float Opacity;
    glm::vec3 Color;
    glm::vec3 GridColor;
    
    Background() {};
    Background(float x, float y, float w, float h, bool border, glm::vec2 gridWidth, glm::vec2 pad);
    
    inline void Move(glm::vec2 pos = glm::vec2(0, 0), bool absolute = false) { Move(pos.x, pos.y, absolute); }
    void Move(float dx = 0, float dy = 0, bool absolute = false);
    void Draw();
    
private:
    float X;
    float Y;
    float Width;
    float Height;
    
    glm::vec2 GridWidth;
    
    Buffer BackgroundBuffer;
    Buffer GridBuffer;
    
    bool GridSet = false;
};

class OrthoElement {
public:
    int Type;
    float Scale;
    
    OrthoElement() {};
    OrthoElement(int type, float x, float y, float scale);
    
    inline void Mesh(int type, glm::vec2 pos) { Mesh(type, pos.x, pos.y); }
    void Mesh(int type, float x, float y);
    void Draw();
    
private:
    Buffer OrthoBuffer;
};

class Interface {
public:
    Interface(){};
    
    void Init();
    
    inline void Set_Document(std::string document) { ActiveDocument = document; }
    float Get_String_Width(std::string string);
    std::vector<std::string> Get_Fitting_String(std::string string, int width);
    
    inline void Add_Text(std::string name, std::string text, glm::vec2 pos) { Add_Text(name, text, pos.x, pos.y); }
    inline void Add_Button(std::string name, std::string text, glm::vec4 dims, Func &function) { Add_Button(name, text, dims.x, dims.y, dims.z, dims.w, function); }
    inline void Add_Slider(std::string name, std::string text, glm::vec4 dims, glm::vec3 range, Func &function) {
        Add_Slider(name, text, dims.x, dims.y, dims.z, dims.w, range.x, range.y, range.z, function); }
    inline void Add_Bar(std::string name, std::string text, glm::vec4 dims, glm::vec3 range) {
        Add_Bar(name, text, dims.x, dims.y, dims.z, dims.w, range.x, range.y, range.z);
    }
    inline void Add_Image(std::string name, std::string path, int texID, glm::vec3 dims) { Add_Image(name, path, texID, dims.x, dims.y, dims.z); }
    inline void Add_Background(std::string name, glm::vec4 dims, bool border = false, glm::vec2 gridWidth = glm::vec2(0), glm::vec2 pad = glm::vec2(0)) {
        Add_Background(name, dims.x, dims.y, dims.z, dims.w, border, gridWidth, pad);
    }
    inline void Add_3D_Element(std::string name, int type, glm::vec2 pos, float scale) { Add_3D_Element(name, type, pos.x, pos.y, scale); }
    
    inline void Add_Text(std::string name, std::string text, float x, float y) {
        TextElements[ActiveDocument].emplace(name, TextElement(text, floor(x), floor(y)));
    }
    inline void Add_Button(std::string name, std::string text, float x, float y, float w, float h, Func &function) {
        Buttons[ActiveDocument].emplace(name, Button(text, x, y, w, h, function));
    }
    inline void Add_Slider(std::string name, std::string text, float x, float y, float w, float h, float min, float max, float value, Func &function) {
        Sliders[ActiveDocument].emplace(name, Slider(text, x, y, w, h, min, max, value, function));
    }
    inline void Add_Bar(std::string name, std::string text, float x, float y, float w, float h, float min, float max, float value) {
        Bars[ActiveDocument].emplace(name, Bar(text, x, y, w, h, min, max, value));
    }
    inline void Add_Image(std::string name, std::string path, int texID, float x, float y, float scale) {
        Images[ActiveDocument].emplace(name, Image(path, texID, x, y, scale));
    }
    inline void Add_Background(std::string name, float x, float y, float w, float h, bool border = false, glm::vec2 gridWidth = glm::vec2(0), glm::vec2 pad = glm::vec2(0)) {
        Backgrounds[ActiveDocument].emplace(name, Background(x, y, w, h, border, gridWidth, pad));
    }
    inline void Add_3D_Element(std::string name, int type, float x, float y, float scale) {
        OrthoElements[ActiveDocument].emplace(name, OrthoElement(type, x, y, scale));
    }
    
    inline void Delete_Text(std::string name) { TextElements[ActiveDocument].erase(name); }
    inline void Delete_Button(std::string name) { Buttons[ActiveDocument].erase(name); }
    inline void Delete_Slider(std::string name) { Sliders[ActiveDocument].erase(name); }
    inline void Delete_Bar(std::string name) { Bars[ActiveDocument].erase(name); }
    inline void Delete_Image(std::string name) { Images[ActiveDocument].erase(name); }
    inline void Delete_Background(std::string name) { Backgrounds[ActiveDocument].erase(name); }
    inline void Delete_3D_Element(std::string name) { OrthoElements[ActiveDocument].erase(name); }
    
    inline TextElement* Get_Text_Element(std::string name) { return &TextElements[ActiveDocument][name]; }
    inline Button* Get_Button(std::string name) { return &Buttons[ActiveDocument][name]; }
    inline Slider* Get_Slider(std::string name) { return &Sliders[ActiveDocument][name]; }
    inline Image* Get_Image(std::string name) { return &Images[ActiveDocument][name]; }
    inline Background* Get_Background(std::string name) { return &Backgrounds[ActiveDocument][name]; }
    inline OrthoElement* Get_3D_Element(std::string name) { return &OrthoElements[ActiveDocument][name]; }
    
    void Mouse_Handler(float x, float y);
    void Click(int mouseButton, int action);
    
    void Draw_Document(std::string document);
    
private:
    std::map<std::string, std::map<std::string, TextElement >> TextElements;
    std::map<std::string, std::map<std::string, Button      >> Buttons;
    std::map<std::string, std::map<std::string, Slider      >> Sliders;
    std::map<std::string, std::map<std::string, Bar         >> Bars;
    std::map<std::string, std::map<std::string, Image       >> Images;
    std::map<std::string, std::map<std::string, Background  >> Backgrounds;
    std::map<std::string, std::map<std::string, OrthoElement>> OrthoElements;
    
    bool Holding = false;
    std::string ActiveDocument = "";
    
    void Init_Shaders();
    void Init_Text();
};