#pragma once

#include "../Variables.h"

class Chunk {
public:
    glm::vec3 Position;
    VBO vbo;

    // int Offset = 0;

    bool IsMeshed = false;
    bool GeneratedFeatures = false;

    std::vector<glm::vec3> Blocks;

    char BlockMap[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE] = {0};
    // int Offsets[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE] = {0};

    Chunk(glm::vec3 position) {
        Position = position;
        this->Generate();
    }

    void Generate() {
        for (int x = 0; x < CHUNK_SIZE; x++) {
            float gx = (Position.x * CHUNK_SIZE + x) / CHUNK_ZOOM;

            for (int y = 0; y < CHUNK_SIZE; y++) {
                float gy = (Position.y * CHUNK_SIZE + y) / CHUNK_ZOOM;

                for (int z = 0; z < CHUNK_SIZE; z++) {
                    float gz = (Position.z * CHUNK_SIZE + z) / CHUNK_ZOOM;

                    double noiseValue = noiseModule.GetValue(gx, gy, gz);

                    if (noiseValue >= 0.0) {
                        this->BlockMap[x][y][z] = 1;
                        this->Blocks.push_back(glm::vec3(x, y, z));
                    }
                }
            }
        }
    }

    void Mesh() {
        std::vector<GLfloat> data;
        int vCount = 0;

        for (int i = 0; i < this->Blocks.size(); i++) {
            this->Add_Block(this->Blocks[i], data);
            vCount += 36;
        }

        GLfloat data_array[vCount * 8];
        std::copy(data.begin(), data.end(), data_array);

        this->vbo.Data(data_array, vCount);
    }

    void Draw() {
        this->vbo.Draw();
    }
private:
    void Add_Block(glm::vec3 pos, std::vector<GLfloat> &block_data) {
        for (int i = 0; i < 36; i++) {
            block_data.push_back(vertices[i][0] + pos.x);
            block_data.push_back(vertices[i][1] + pos.y);
            block_data.push_back(vertices[i][2] + pos.z);

            for (int j = 3; j < 8; j++) {
                block_data.push_back(vertices[i][j]);
            }
        }
    }
};