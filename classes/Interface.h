#pragma once

#include "Buffer.h"

typedef void (Func)(void);

extern int SCREEN_WIDTH;
extern int SCREEN_HEIGHT;

extern float vertices[6][6][3];
extern float tex_coords[6][6][2];

extern std::map<unsigned int, glm::vec2> textureCoords;
extern std::map<unsigned int, std::vector<glm::vec2>> MultiTextures;
extern std::map<unsigned int, std::vector<std::vector<glm::vec2>>> CustomTexCoords;
extern std::map<unsigned int, std::vector<std::vector<glm::vec3>>> CustomVertices;

Data Get_3D_Mesh(unsigned int type, float x, float y, bool offsets = false);

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
    Button(std::string text, float x, float y, float w, Func &function);
    
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
    Slider(std::string text, float x, float y, float w, float min, float max, float value, Func &function);
    
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
    
    Background() {};
    Background(float x, float y, float w, float h, bool border, glm::vec2 gridWidth);
    
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
    
    void Add_Text(std::string name, std::string text, float x, float y);
    void Add_Button(std::string name, std::string text, float x, float y, float w, Func &function);
    void Add_Slider(std::string name, std::string text, float x, float y, float w, float min, float max, float value, Func &function);
    void Add_Background(std::string name, float x, float y, float w, float h, bool border = false, glm::vec2 gridWidth = glm::vec2(0, 0));
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
    std::string ActiveDocument = "";
    
    void Init_Shaders();
    void Init_Text();
};