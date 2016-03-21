#pragma once

#include <vector>

class VBO {
public:
    VBO();
    ~VBO();

    void Data(std::vector<float> data);
    void Draw();

private:
    int vertexCount;
    unsigned int VertexArrayObject;
    unsigned int VertexBufferObject;
};