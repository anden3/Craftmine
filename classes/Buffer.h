#pragma once

#include <vector>
#include <map>

#include "Shader.h"

typedef std::vector<float> Data;

class Buffer {
public:
    int VertexType = GL_TRIANGLES;
    
    void Init(Shader *shader);
    void Create(const std::vector<int> &config, const Data &data = Data {});
    void Upload(const Data &data, int start = 0, bool sub = false);
    
    void Draw(int start = 0, int length = 0);
    
private:
    Shader* BufferShader;
    
    unsigned int VAO;
    unsigned int VBO;
    
    unsigned int VertexSize;
    int Vertices = 0;
};