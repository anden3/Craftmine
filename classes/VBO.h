#pragma once

#include "../Variables.h"

class VBO {
public:
    int vertexCount = 0;

    VBO(int size=0) {
        if (size != 0) {
            this->bufferSize = size;
        }

        glGenVertexArrays(1, &this->VertexArrayObject);
        glGenBuffers(1, &this->VertexBufferObject);

        glBindBuffer(GL_ARRAY_BUFFER, this->VertexBufferObject);
        glBufferData(GL_ARRAY_BUFFER, this->bufferSize, NULL, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void Data(GLfloat data[], int vCount) {
        glBindVertexArray(this->VertexArrayObject);

        glBindBuffer(GL_ARRAY_BUFFER, this->VertexBufferObject);
        glBufferData(GL_ARRAY_BUFFER, vCount * 8 * sizeof(GLfloat), data, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        this->vertexCount += vCount;
    }

    void Draw() {
        glBindVertexArray(this->VertexArrayObject);
        glDrawArrays(GL_TRIANGLES, 0, this->vertexCount);
        glBindVertexArray(0);
    }

private:
    int bufferSize = pow(CHUNK_SIZE, 3) * 36 * 8;
    GLuint VertexArrayObject, VertexBufferObject;
};