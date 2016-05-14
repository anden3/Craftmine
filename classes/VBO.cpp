#include "VBO.h"

#include <math.h>

VBO::VBO() {
    glGenVertexArrays(1, &VertexArrayObject);
    glGenBuffers(1, &VertexBufferObject);
    
    glBindVertexArray(VertexArrayObject);
    glBindBuffer(GL_ARRAY_BUFFER, VertexBufferObject);
    
    int sizes[4] = {3, 2, 1, 1};
    int size = 0;
    
    for (int i = 0; i < 4; i++) {
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, sizes[i], GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(size * sizeof(float)));
        size += sizes[i];
    }
    
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

VBO::~VBO() {
    glDeleteBuffers(1, &VertexBufferObject);
    glDeleteVertexArrays(1, &VertexArrayObject);
}

void VBO::Data(std::vector<float> data) {
    glBindBuffer(GL_ARRAY_BUFFER, VertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    vertexCount = int(data.size() / 7);
}

void VBO::Draw() {
    if (vertexCount > 0) {
        glBindVertexArray(VertexArrayObject);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        glBindVertexArray(0);
    }
}