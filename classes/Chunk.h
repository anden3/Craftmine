#pragma once

#include "../Variables.h"
#include "../Functions.h"

class Chunk {
public:
    glm::vec3 Position;
    std::vector<glm::vec3> Blocks;

    bool IsMeshed = false;
    bool GeneratedFeatures = false;

    char BlockMap[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE] = {0};

    Chunk(glm::vec3 position) {
        Position = position;
        Generate();
        Mesh();
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
                        BlockMap[x][y][z] = 1;
                        Blocks.push_back(glm::vec3(x, y, z));
                    }
                }
            }
        }
    }

    void Mesh() {
        std::vector<GLfloat> data;
        std::vector<glm::vec3>::iterator block = Blocks.begin();

        while (block != Blocks.end()) {
            std::vector<int> neighbors = Get_Neighbors(*block);
            int neighborSum = 0;

            for (int i = 0; i < 6; i++) {
                neighborSum += neighbors[i];
            }

            if (neighborSum == 6) {
                block = Blocks.erase(block);
            }
            else {
                for (int i = 0; i < 6; i++) {
                    if (neighbors[i] == 0) {
                        for (int j = i * 6; j < i * 6 + 6; j++) {
                            data.push_back(vertices[j][0] + Position.x * CHUNK_SIZE + block->x);
                            data.push_back(vertices[j][1] + Position.y * CHUNK_SIZE + block->y);
                            data.push_back(vertices[j][2] + Position.z * CHUNK_SIZE + block->z);

                            for (int k = 3; k < 8; k++) {
                                data.push_back(vertices[j][k]);
                            }
                        }
                    }
                }
                ++block;
            }
        }

        vbo.Data(data);
    }

    void Draw() {
        vbo.Draw();
    }
private:
    VBO vbo;

    std::vector<int> Get_Neighbors(glm::vec3 pos) {
        std::vector<int> result;

        int x = pos.x;
        int y = pos.y;
        int z = pos.z;

        glm::vec3 Neighbors[6] = {
                glm::vec3(x - 1, y, z), glm::vec3(x + 1, y, z),
                glm::vec3(x, y - 1, z), glm::vec3(x, y + 1, z),
                glm::vec3(x, y, z - 1), glm::vec3(x, y, z + 1),
        };

        for (int i = 0; i < 6; i++) {
            glm::vec3 neighbor = Neighbors[i];

            if (neighbor.x < 0 || neighbor.x >= CHUNK_SIZE) {
                result.push_back(0);
                continue;
            }

            if (neighbor.y < 0) {
                result.push_back(1);
                continue;
            }

            else if (neighbor.y >= CHUNK_SIZE) {
                result.push_back(0);
                continue;
            }

            if (neighbor.z < 0 || neighbor.z >= CHUNK_SIZE) {
                result.push_back(0);
                continue;
            }

            int nx = neighbor.x;
            int ny = neighbor.y;
            int nz = neighbor.z;

            if (BlockMap[nx][ny][nz] >= 1) {
                result.push_back(1);
            }
            else {
                result.push_back(0);
            }
        }

        return result;
    }
};