#pragma once

#include <map>
#include <string>
#include <vector>

#define GLM_SWIZZLE
#include <glm/glm.hpp>

typedef std::vector<float> Data;
class Shader;

class Buffer {
public:
    int VertexType;
    int Vertices = 0;
    
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
    
    void Create(const std::vector<int> &config, const Data &data = Data {});
};

class UniformBuffer {
public:
    void Create(std::string name, int bufferID, int size, std::vector<Shader*> shaders);
    void Add(std::string name, int bufferID, Shader* shader);
    
    template <typename T>
    void Upload(int index, T t);
    void Upload(int index, glm::mat4 matrix);

private:
    unsigned int UBO;
    int BufferID;
};