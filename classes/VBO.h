#pragma once

#include <vector>

#include <GLEW/glew.h>
#include <GLFW/glfw3.h>

class VBO {
public:
    unsigned int VertexArrayObject;
    unsigned int VertexBufferObject;

    VBO();
    ~VBO();

    void Data(std::vector<float> data);
    void Draw();

private:
    int vertexCount;
};