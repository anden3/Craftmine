#include "Buffer.h"

#include "Shader.h"

static const int FLOAT_SIZE = static_cast<int>(sizeof(float));

void Buffer::Init(Shader *shader) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    VertexType = GL_TRIANGLES;
    BufferShader = shader;
}

void Buffer::Create(const std::vector<int> &config, const Data &data) {
    VertexSize = 0;

    for (int const &element : config) {
        VertexSize += static_cast<unsigned int>(element);
    }

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    if (data.size() > 0) {
        glBufferData(
            GL_ARRAY_BUFFER, static_cast<long>(data.size() * sizeof(float)),
            data.data(), GL_STATIC_DRAW
        );
        Vertices = static_cast<int>(data.size()) / VertexSize;
    }

    unsigned int index = 0;
    int partSum = 0;

    for (int const &element : config) {
        glEnableVertexAttribArray(index);
        glVertexAttribPointer(index, element, GL_FLOAT, false, VertexSize * FLOAT_SIZE, reinterpret_cast<const GLvoid*>(partSum * FLOAT_SIZE));

        partSum += element;
        ++index;
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Buffer::Upload(const Data &data, int start, bool sub) {
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    if (start == 0 && !sub) {
        Vertices = static_cast<int>(data.size()) / VertexSize;
        glBufferData(
            GL_ARRAY_BUFFER, static_cast<long>(data.size() * sizeof(float)),
            data.data(), GL_STATIC_DRAW
        );
    }
    else {
        glBufferSubData(
            GL_ARRAY_BUFFER, start,
            static_cast<long>(data.size() * sizeof(float)), data.data()
        );
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Buffer::Draw(int start, int length) {
    if (Vertices == 0) {
        return;
    }

    if (length == 0) {
        length = Vertices;
    }

    BufferShader->Bind();

    glBindVertexArray(VAO);
    glDrawArrays(static_cast<unsigned int>(VertexType), start, length);
    glBindVertexArray(0);

    BufferShader->Unbind();
}

void UniformBuffer::Create(std::string name, unsigned int bufferID, int size, std::vector<Shader*> shaders) {
    BufferID = bufferID;

    glGenBuffers(1, &UBO);

    for (auto const &shader : shaders) {
        glUniformBlockBinding(shader->Program, glGetUniformBlockIndex(shader->Program, name.c_str()), bufferID);
    }

    glBindBuffer(GL_UNIFORM_BUFFER, UBO);
    glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferRange(GL_UNIFORM_BUFFER, 0, UBO, 0, size);
}

void UniformBuffer::Add(std::string name, unsigned int bufferID, Shader* shader) {
    glUniformBlockBinding(shader->Program, glGetUniformBlockIndex(shader->Program, name.c_str()), bufferID);
}

template <typename T>
void UniformBuffer::Upload(unsigned long index, T t) {
    glBindBuffer(GL_UNIFORM_BUFFER, UBO);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(T) * index, sizeof(T), t);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBuffer::Upload(unsigned long index, glm::mat4 matrix) {
    glBindBuffer(GL_UNIFORM_BUFFER, UBO);
    glBufferSubData(
        GL_UNIFORM_BUFFER, static_cast<long>(sizeof(glm::mat4) * index),
        sizeof(glm::mat4), glm::value_ptr(matrix)
    );
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
