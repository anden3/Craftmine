#pragma once

#include "Buffer.h"

typedef void (Func)(void);

extern int SCREEN_WIDTH;
extern int SCREEN_HEIGHT;

extern unsigned int IMAGE_SIZE_X;
extern unsigned int IMAGE_SIZE_Y;

extern float vertices[6][6][3];
extern float tex_coords[6][6][2];

extern std::map<unsigned int, glm::vec2> textureCoords;
extern std::map<unsigned int, std::vector<glm::vec2>> MultiTextures;
extern std::map<unsigned int, std::vector<std::vector<glm::vec2>>> CustomTexCoords;
extern std::map<unsigned int, std::vector<std::vector<glm::vec3>>> CustomVertices;

Data Get_3D_Mesh(unsigned int type, float x, float y, bool offsets = false);

template <typename T>
inline float X_Frac(const T a, const T b) {
    return SCREEN_WIDTH * float(a) / float(b);
}

template <typename T>
inline float Y_Frac(const T a, const T b) {
    return SCREEN_HEIGHT * float(a) / float(b);
}

template <typename T>
inline float X_Per(const T percentage) {
    return SCREEN_WIDTH * float(percentage) / 100.0f;
}

template <typename T>
inline float Y_Per(const T percentage) {
    return SCREEN_HEIGHT * float(percentage) / 100.0f;
}

class TextElement {
public:
    std::string Text;
    
    float X;
    float Y;
    float Scale;
    float Opacity;
    
    float Height;
    float Width;
    
    glm::vec3 Color;
    
    TextElement() {};
    
    void Create(std::string text, float x, float y, float opacity = 1.0f, glm::vec3 color = glm::vec3(1.0f), float scale = 1.0f);
    void Center(float x, float y, float width);
    void Draw();
    
private:
    float Get_Width();
};

class Button {
public:
    float X;
    float Y;
    float Width;
    float Height;
    
    TextElement Text;
    
    Button() {};
    Button(std::string text, float x, float y, float w, float h, Func &function);
    
    inline void Hover();
    inline void Stop_Hover();
    
    inline void Press();
    inline void Release();
    
    void Draw();
    
private:
    float Opacity;
    glm::vec3 Color;
    
    bool Active = true;
    bool IsHovering = false;
    
    Buffer BackgroundBuffer;
    Buffer BorderBuffer;
    
    Func* Function;
};

class Slider {
public:
    float X;
    float Y;
    float Width;
    float Height;
    
    float HandlePosition;
    float Value;
    
    TextElement Text;
    
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
    
    float Opacity;
    float HandleOpacity;
    
    glm::vec3 Color;
    glm::vec3 HandleColor;
    
    bool Active = true;
    bool IsHovering = false;
    
    Buffer BackgroundBuffer;
    Buffer BorderBuffer;
    Buffer HandleBuffer;
    
    Func* Function;
};

class Background {
public:
    float Opacity;
    glm::vec3 Color;
    glm::vec3 GridColor;
    
    Background() {};
    Background(float x, float y, float w, float h, bool border, glm::vec2 gridWidth, glm::vec2 pad);
    
    inline void Move(glm::vec2 pos = glm::vec2(0, 0), bool absolute = false) {
        Move(pos.x, pos.y, absolute);
    }
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
    int Get_Fitting_String(std::string string, int width);
    
    inline void Add_Text(std::string name, std::string text, glm::vec2 pos) { Add_Text(name, text, pos.x, pos.y); }
    inline void Add_Button(std::string name, std::string text, glm::vec4 dims, Func &function) { Add_Button(name, text, dims.x, dims.y, dims.z, dims.w, function); }
    inline void Add_Slider(std::string name, std::string text, glm::vec4 dims, glm::vec3 range, Func &function) {
        Add_Slider(name, text, dims.x, dims.y, dims.z, dims.w, range.x, range.y, range.z, function); }
    inline void Add_Background(std::string name, glm::vec4 dims, bool border = false, glm::vec2 gridWidth = glm::vec2(0), glm::vec2 pad = glm::vec2(0)) {
        Add_Background(name, dims.x, dims.y, dims.z, dims.w, border, gridWidth, pad);
    }
    inline void Add_3D_Element(std::string name, int type, glm::vec2 pos, float scale) { Add_3D_Element(name, type, pos.x, pos.y, scale); }
    
    void Add_Text(std::string name, std::string text, float x, float y);
    void Add_Button(std::string name, std::string text, float x, float y, float w, float h, Func &function);
    void Add_Slider(std::string name, std::string text, float x, float y, float w, float h, float min, float max, float value, Func &function);
    void Add_Background(std::string name, float x, float y, float w, float h, bool border = false, glm::vec2 gridWidth = glm::vec2(0, 0), glm::vec2 pad = glm::vec2(0, 0));
    void Add_3D_Element(std::string name, int type, float x, float y, float scale);
    
    void Delete_Text(std::string name);
    void Delete_Button(std::string name);
    void Delete_Slider(std::string name);
    void Delete_Background(std::string name);
    void Delete_3D_Element(std::string name);
    
    TextElement* Get_Text_Element(std::string name);
    Button* Get_Button(std::string name);
    Slider* Get_Slider(std::string name);
    Background* Get_Background(std::string name);
    OrthoElement* Get_3D_Element(std::string name);
    
    void Mouse_Handler(float x, float y);
    void Click(int mouseButton, int action);
    
    void Draw_Document(std::string document);
    
private:
    bool Holding = false;
    std::string ActiveDocument = "";
    
    void Init_Shaders();
    void Init_Text();
};