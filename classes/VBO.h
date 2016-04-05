#pragma once

#include <vector>

#include <GL/glew.h>
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