#pragma once

#include <string>

#include "Buffer.h"

#define EXPAND_VEC2(V) V.x, V.y
#define EXPAND_VEC3(V) V.x, V.y, V.z
#define EXPAND_VEC4(V) V.x, V.y, V.z, V.w

typedef void (Func)(void*);
typedef std::vector<float> Data;

struct Block;

inline std::string Format_Vector(glm::vec3 vector) {
    return std::string(
        "X: " + std::to_string(int(vector.x)) + "\t\t" +
        "Y: " + std::to_string(int(vector.y)) + "\t\t" +
        "Z: " + std::to_string(int(vector.z))
    );
}

template <typename T>
inline bool In_Range(T value, glm::dvec2 bounds) {
    return value >= bounds.x && value <= (bounds.x + bounds.y);
}

template <typename T>
inline Data Get_Rect(T x1, T x2, T y1, T y2) {
    return Data {x1, y1, x2, y1, x2, y2, x1, y1, x2, y2, x1, y2};
}

template <typename T>
inline Data Get_Border(T x1, T x2, T y1, T y2) {
    return Data {
        x1, y1, x2, y1, x2, y1, x2, y2, x2, y2,
        x1, y2, x1, y2, x1, y1, x1 - 0.5f, y2, x1, y2
    };
}

template <typename T>
inline Data Get_Tex_Rect(T x1, T x2, T y1, T y2) {
    return Data {
        x1, y1, 0, 1, x2, y1, 1, 1, x2, y2, 1, 0,
        x1, y1, 0, 1, x2, y2, 1, 0, x1, y2, 0, 0
    };
}

Data Get_3D_Mesh(const Block* block, float x, float y, bool offsets = false);

std::tuple<unsigned int, int, int> Load_Texture(std::string file, bool mipmap = false);
unsigned int Load_Array_Texture(std::string file, glm::ivec2 subCount, int mipmap = 0);

void Take_Screenshot();

float Scale_X(const float x);
float Scale_Y(const float y);
glm::vec2 Scale(const float t);
glm::vec2 Scale(const float x, const float y);

template <typename V, typename T>
inline void Extend(std::vector<V> &storage, T t) { storage.push_back(V(t)); }

template <typename V, typename T, typename... Args>
inline void Extend(std::vector<V> &storage, T t, Args... args) {
    storage.push_back(V(t));
    Extend(storage, args...);
}

template <typename T>
inline void Extend(std::vector<T> &storage, std::vector<T> input) {
    for (T const &element : input) {
        storage.push_back(element);
    }
}

template <typename T>
inline void Extend(std::vector<T> &storage, glm::vec2 input) {
    Extend(storage, T(input.x), T(input.y));
}

template <typename T>
inline void Extend(std::vector<T> &storage, glm::vec3 input) {
    Extend(storage, T(input.x), T(input.y), T(input.z));
}

template <typename T>
inline void Extend(std::vector<T> &storage, glm::vec4 input) {
    Extend(storage, T(input.x), T(input.y), T(input.z), T(input.w));
}

class TextElement {
public:
    std::string Name;
    std::string Text = "";

    float X;
    float Y;
    float Scale;
    float Opacity;

    float Height;
    float Width;

    glm::vec3 Color;

    TextElement() {}
    TextElement(std::string name, std::string text, float x, float y) { Create(name, text, x, y); }

    void Create(std::string name, std::string text, float x, float y, float opacity = 1.0f,
        glm::vec3 color = {1, 1, 1}, float scale = 1.0f);

    inline void Center(glm::vec2 pos, float width, glm::bvec2 axes = {true, true}) {
        Center(pos.x, pos.y, width, axes);
    }
    void Center(float x, float y, float width, glm::bvec2 axes = {true, true});

    void Set_Text(std::string newText);

    void Mesh();
    void Draw();

private:
    Buffer TextBuffer;

    float OriginalX = 0;
    float OriginalY = 0;
    float CenterWidth = 0;
    glm::bvec2 Centered = {false, false};

    float Get_Width();
};

class UIElement {
public:
    std::string Name;

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
    Button() {}
    Button(std::string name, std::string text, float x, float y, float w, float h, Func &function);

    inline void Hover();
    inline void Stop_Hover();

    inline void Press();
    inline void Release();
};

class Slider : public UIElement {
public:
    float HandlePosition;
    float Value;

    Slider() {}
    Slider(std::string name, std::string text, float x, float y, float w, float h, float min, float max, float value, Func &function);

    inline void Hover();
    inline void Stop_Hover();

    inline void Press();
    void Release();

    void Move(float position, bool setValue = false);
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

    Bar() {}
    Bar(std::string name, std::string text, float x, float y, float w, float h, float min, float max, float value);

    void Move(float value);
    void Draw();

private:
    float Min;
    float Max;

    Buffer BarBuffer;
};

class Image : public UIElement {
public:
    Image() {}
    Image(std::string name, std::string file, int texID, float x, float y, float scale);

    void Center();
    void Draw();

private:
    float Scale;

    unsigned int Texture;
    int TexID;

    Buffer ImageBuffer;
};

class Background : public UIElement {
public:
    glm::vec3 GridColor;

    Background() {}
    Background(std::string name, float x, float y, float w, float h, bool border, glm::vec2 gridWidth = {0, 0}, glm::vec2 pad = {0, 0});

    inline void Move(glm::vec2 pos = glm::vec2(0, 0), bool absolute = false) { Move(pos.x, pos.y, absolute); }
    void Move(float dx = 0, float dy = 0, bool absolute = false);
    void Draw();

private:
    glm::vec2 GridWidth;
    Buffer GridBuffer;

    bool GridSet = false;
};

class TextBox : public UIElement {
public:
    bool Visible = true;
    std::string Text = "";

    TextBox() {}
    TextBox(std::string name, float x, float y, float w, float h);

    void Set_Cursor_Visibility(bool cursorVisible);

    void Key_Handler(int key);

    void Input(unsigned int codepoint);
    void Draw();

    void Clear();

private:
    Background BG;
    TextElement TextEl;
    TextElement Cursor;

    unsigned long CursorPos = 0;
    bool CursorVisible = false;

    float TextWidth = 0.0f;
    float MaxWidth = 0.0f;

    void Update();
};

class OrthoElement {
public:
    std::string Name;

    int Type;
    float Scale;

    OrthoElement() {}
    OrthoElement(std::string name, int type, int data, float x, float y, float scale);

    inline void Mesh(int type, int data, glm::vec2 pos) { Mesh(type, data, pos.x, pos.y); }
    void Mesh(int type, int data, float x, float y);
    void Draw();

private:
    glm::mat4 ModelMatrix;
    Buffer OrthoBuffer;
};

namespace Interface {
    extern bool Holding;
    extern void* HoveringElement;
    extern std::string HoveringType;
    extern std::string ActiveDocument;

    void Init();
    void Init_Text();
    void Init_Shaders();

    void Mouse_Handler(double x, double y);
    void Click(int mouseButton, int action);

    void Set_Document(std::string document);
    void Draw_Document(std::string document);

    float Get_String_Width(std::string string);
    std::vector<std::string> Get_Fitting_String(std::string string, int width);

    void Add_Text      (std::string name, std::string text, float x, float y);
    void Add_Text_Box  (std::string name, float x, float y, float w, float h);
    void Add_3D_Element(std::string name, int type, int data, float x, float y, float scale);
    void Add_Image     (std::string name, std::string path, int texID, float x, float y, float scale);
    void Add_Button    (std::string name, std::string text, float x, float y, float w, float h, Func &function);
    void Add_Bar       (std::string name, std::string text, float x, float y, float w, float h, float min, float max, float value);
    void Add_Background(std::string name, glm::vec4 dims, bool border = false, glm::vec2 gridWidth = {0, 0}, glm::vec2 pad = {0, 0});
    void Add_Slider    (std::string name, std::string text, float x, float y, float w, float h, float min, float max, float value, Func &function);

    inline void Add_Text(std::string name, std::string text, glm::vec2 pos) {
        Add_Text(name, text, EXPAND_VEC2(pos));
    }

    inline void Add_Text_Box(std::string name, glm::vec4 dims) {
        Add_Text_Box(name, EXPAND_VEC4(dims));
    }

    inline void Add_Button(std::string name, std::string text, glm::vec4 dims, Func &function) {
        Add_Button(name, text, EXPAND_VEC4(dims), function);
    }

    inline void Add_Slider(std::string name, std::string text, glm::vec4 dims, glm::vec3 range, Func &function) {
        Add_Slider(name, text, EXPAND_VEC4(dims), EXPAND_VEC3(range), function);
    }

    inline void Add_Bar(std::string name, std::string text, glm::vec4 dims, glm::vec3 range) {
        Add_Bar(name, text, EXPAND_VEC4(dims), EXPAND_VEC3(range));
    }

    inline void Add_Image(std::string name, std::string path, int texID, glm::vec3 dims) {
        Interface::Add_Image(name, path, texID, EXPAND_VEC3(dims));
    }

    inline void Add_3D_Element(std::string name, int type, int data, glm::vec2 pos, float scale) {
        Add_3D_Element(name, type, data, EXPAND_VEC2(pos), scale);
    }

    void Delete_Bar       (std::string name);
    void Delete_Text      (std::string name);
    void Delete_Image     (std::string name);
    void Delete_Button    (std::string name);
    void Delete_Slider    (std::string name);
    void Delete_Text_Box  (std::string name);
    void Delete_Background(std::string name);
    void Delete_3D_Element(std::string name);

    Image*        Get_Image       (std::string name);
    Button*       Get_Button      (std::string name);
    Slider*       Get_Slider      (std::string name);
    TextBox*      Get_Text_Box    (std::string name);
    Background*   Get_Background  (std::string name);
    OrthoElement* Get_3D_Element  (std::string name);
    TextElement*  Get_Text_Element(std::string name);
};
