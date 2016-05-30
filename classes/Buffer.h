#pragma once

#include <map>
#include <string>
#include <vector>

#define GLM_SWIZZLE
#include <glm/glm.hpp>

typedef std::vector<float> Data;
class Shader;

struct Block {
    std::string Name = "";
    std::string Sound = "";
    std::string Data = "";
    
    unsigned int ID = 0;
    float Hardness = 0;
    
    bool Transparent = false;
    bool Collision = true;
    bool Targetable = true;
    bool HasIcon = false;
    bool HasTexture = false;
    bool MultiTextures = false;
    bool CustomTexCoords = false;
    bool CustomVertices = false;
    
    int Luminosity = 0;
    
    glm::vec2 Icon = glm::vec2(0, 0);
    glm::vec2 Texture = glm::vec2(0, 0);
    
    std::vector<glm::vec2> Textures = {};
    std::vector<std::vector<glm::vec2>> TexCoords = {};
    std::vector<std::vector<glm::vec3>> Vertices = {};
    
    std::map<std::string, Block> Types = {};
};

extern std::map<unsigned int, Block> BlockTypes;

Block* Get_Block_Type(unsigned int type, std::string data = "");

class Buffer {
public:
    int VertexType;
    Shader* BufferShader;
    
    inline void Create(const int &a, const Data &data = Data {}) { Create(std::vector<int> {a}, data); }
    inline void Create(const int &a, const int &b, const Data &data = Data {}) { Create(std::vector<int> {a, b}, data); }
    inline void Create(const int &a, const int &b, const int &c, const Data &data = Data {}) { Create(std::vector<int> {a, b, c}, data); }
    inline void Create(const int &a, const int &b, const int &c, const int &d, const Data &data = Data {}) { Create(std::vector<int> {a, b, c, d}, data); }
    inline void Create(const int &a, const int &b, const int &c, const int &d, const int &e, const Data &data = Data {}) {Create(std::vector<int> {a, b, c, d, e}, data);}
    
    void Init(Shader *shader);
    void Upload(const Data &data, int start = 0, bool sub = false);
    void Draw(int start = 0, int length = 0);
    
private:
    unsigned int VAO;
    unsigned int VBO;
    
    unsigned int VertexSize;
    int Vertices = 0;
    
    void Create(const std::vector<int> &config, const Data &data = Data {});
};

class UniformBuffer {
public:
    void Create(std::string name, int bufferID, int size, std::vector<Shader*> shaders);
    
    template <typename T>
    void Upload(int index, T t);
    void Upload(int index, glm::mat4 matrix);

private:
    unsigned int UBO;
    int BufferID;
};