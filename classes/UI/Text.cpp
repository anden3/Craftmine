#include "Text.h"

#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

#include <glm/gtc/matrix_transform.hpp>

#include "Buffer.h"

struct Character {
	unsigned int TextureID;
	glm::ivec2 Size;
	glm::ivec2 Bearing;
	unsigned int Advance;
};

struct Group {
    float X = -1;
    float Y = -1;
    float Scale = -1;
    float Opacity = -1;
    
    glm::vec3 Color = glm::vec3(-1);
    
    std::map<std::string, String> Strings;
};

float X = 0;
float Y = 0;

float Scale = 1.0f;
float Opacity = 1.0f;

glm::vec3 Color = glm::vec3(1.0f);

std::string currentGroup = "default";

const int TEXT_TEXTURE_UNIT = 10;
Buffer TextBuffer;

Shader* textShader;

std::map<char, Character> Characters;
std::map<std::string, Group> Groups;

void Text::Init(std::string font, int font_size) {
    textShader = new Shader("text");
    
    FT_Library ft;
    FT_Face face;
    
    FT_Init_FreeType(&ft);
    FT_New_Face(ft, std::string("fonts/" + font + ".ttf").c_str(), 0, &face);
    FT_Set_Pixel_Sizes(face, 0, font_size);
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    for (GLubyte c = 0; c < 128; c++) {
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
        
        Characters.insert(std::pair<GLchar, Character>(c, character));
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    
    Data data;
    data.resize(4 * 6);
    
    TextBuffer.Init(textShader);
    TextBuffer.Create(std::vector<int> {4}, data);
    
    textShader->Upload("projection", glm::ortho(0.0f, (float)SCREEN_WIDTH, 0.0f, (float)SCREEN_HEIGHT));
    textShader->Upload("text", TEXT_TEXTURE_UNIT);
}

void Text::Add(std::string name, std::string text, float y) {
    float yVal = y;
    
    if (y == -1) yVal = Y;
    
    String string;
    
    string.Text = text;
    string.Y = yVal;
    string.Height = FONT_SIZE * Scale;
    
    string.Width = Get_Width(text);
    
    Groups[currentGroup].Strings[name] = string;
}

void Text::Remove(std::string name) {
    Groups[currentGroup].Strings.erase(name);
}

void Text::Delete_Group(std::string group) {
    if (currentGroup == group) {
        currentGroup = "default";
    }
    
    Groups.erase(group);
}

float Text::Get_Width(std::string string) {
    return Groups[currentGroup].Strings[string].Width;
}

float Text::Get_String_Width(std::string string) {
    float width = 0;
    std::string::const_iterator c;
    
    for (c = string.begin(); c != string.end(); c++) {
        width += (Characters[*c].Advance >> 6) * Scale;
    }
    
    return width;
}

std::string Text::Get_String_To_Width(std::string string, float width) {
    float currentWidth = 0;
    int index = 1;
    
    std::string::const_iterator c;
    
    for (c = string.begin(); c != string.end(); c++) {
        currentWidth += (Characters[*c].Advance >> 6) * Scale;
        
        if (currentWidth > width) {
            return string.substr(0, index - 1);
        }
        index++;
    }
    
    return string;
}

float Text::Get_Opacity(std::string name) {
    if (name == "global") return Opacity;
    if (Groups[currentGroup].Strings.count(name)) return Groups[currentGroup].Strings[name].Opacity;
    return Groups[name].Opacity;
}

void Text::Set_Group(std::string group) {
    currentGroup = group;
}

void Text::Unset_Group() {
    currentGroup = "default";
}

void Text::Set_Text(std::string name, std::string text) {
    Groups[currentGroup].Strings[name].Text = text;
}

void Text::Set_X(std::string name, float x) {
    if (name == "global") X = x;
    else if (Groups[currentGroup].Strings.count(name)) Groups[currentGroup].Strings[name].X = x;
    else Groups[name].X = x;
}

void Text::Set_Y(std::string name, float y) {
    if (name == "global") Y = y;
    else if (Groups[currentGroup].Strings.count(name)) Groups[currentGroup].Strings[name].Y = y;
    else Groups[name].Y = y;
}

void Text::Set_Scale(std::string name, float scale) {
    if (name == "global") Scale = scale;
    else if (Groups[currentGroup].Strings.count(name)) Groups[currentGroup].Strings[name].Scale = scale;
    else Groups[name].Scale = scale;
}

void Text::Set_Opacity(std::string name, float opacity) {
    if (name == "global") Opacity = opacity;
    else if (Groups[currentGroup].Strings.count(name)) Groups[currentGroup].Strings[name].Opacity = opacity;
    else Groups[name].Opacity = opacity;
}

void Text::Set_Color(std::string name, glm::vec3 color) {
    if (name == "global") Color = color;
    else if (Groups[currentGroup].Strings.count(name)) Groups[currentGroup].Strings[name].Color = color;
    else Groups[name].Color = color;
}

void Text::Draw(String string) {
    float x = string.X;
    float y = string.Y;
    float scale = string.Scale;
    float opacity = string.Opacity;
    glm::vec3 color = string.Color;
    
    if (x == -1) x = X;
    if (y == -1) y = Y;
    if (scale == -1) scale = Scale;
    if (opacity == -1) opacity = Opacity;
    if (color == glm::vec3(-1)) color = Color;
    
    textShader->Upload("textColor", glm::vec4(color, opacity));
    
    glActiveTexture(GL_TEXTURE0 + TEXT_TEXTURE_UNIT);
    
    std::string::const_iterator c;
    
    for (c = string.Text.begin(); c != string.Text.end(); c++) {
        Character ch = Characters[*c];
        
        float xPos = x + ch.Bearing.x * scale;
        float yPos = y - (ch.Size.y - ch.Bearing.y) * scale;
        
        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        
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
        
        x += (ch.Advance >> 6) * scale;
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    textShader->Unbind();
}

void Text::Draw_String(std::string name) {
    Text::Draw(Groups[currentGroup].Strings[name]);
}

void Text::Draw_Group(std::string group) {
    float oldX = X;
    float oldY = Y;
    float oldScale = Scale;
    float oldOpacity = Opacity;
    glm::vec3 oldColor = Color;
    
    if (Groups[group].X       != -1)                  X = Groups[group].X;
    if (Groups[group].Y       != -1)                  Y = Groups[group].Y;
    if (Groups[group].Scale   != -1)              Scale = Groups[group].Scale;
    if (Groups[group].Opacity != -1)            Opacity = Groups[group].Opacity;
    if (Groups[group].Color   != glm::vec3(-1))   Color = Groups[group].Color;
    
    for (auto const string : Groups[group].Strings) {
        Draw(string.second);
    }
    
    X = oldX;
    Y = oldY;
    Scale = oldScale;
    Opacity = oldOpacity;
    Color = oldColor;
}