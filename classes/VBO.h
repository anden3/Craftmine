#pragma once

#include <vector>

class VBO {
public:
    VBO();

    void Data(std::vector<float> data);
    void Draw();

private:
    //int bufferSize;
    int vertexCount;
    unsigned int VertexArrayObject;
    unsigned int VertexBufferObject;
};