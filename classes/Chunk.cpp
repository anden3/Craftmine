#include "Chunk.h"

#include <noise/noise.h>

static const int CHUNK_ZOOM = 50;

static const float NOISE_DENSITY_BLOCK = 0.5f;

noise::module::Perlin noiseModule;

enum Directions {
    LEFT,
    RIGHT,
    DOWN,
    UP,
    BACK,
    FRONT
};

float vertices[36][8] = {
        // Left
        { 0.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f},
        { 0.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f},
        { 0.0f,  1.0f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f},

        { 0.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f},
        { 0.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f},
        { 0.0f,  0.0f,  1.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f},

        // Right
        { 1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f},
        { 1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f},
        { 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f},

        { 1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f},
        { 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f},
        { 1.0f,  0.0f,  1.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f},

        // Down
        { 0.0f,  0.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f},
        { 1.0f,  0.0f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f},
        { 1.0f,  0.0f,  1.0f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f},

        { 1.0f,  0.0f,  1.0f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f},
        { 0.0f,  0.0f,  1.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f},
        { 0.0f,  0.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f},

        // Up
        { 0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f},
        { 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f},
        { 1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f},

        { 0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f},
        { 0.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f},
        { 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f},

        // Back
        { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f},
        { 1.0f,  1.0f,  0.0f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f},
        { 1.0f,  0.0f,  0.0f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f},

        { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f},
        { 0.0f,  1.0f,  0.0f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f},
        { 1.0f,  1.0f,  0.0f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f},

        // Front
        { 0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f},
        { 1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f},
        { 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f},

        { 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f},
        { 0.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f},
        { 0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f}
    };

Chunk::Chunk(glm::vec3 position) {
    Position = position;
}

void Chunk::Generate() {
    noiseModule.SetPersistence(0.5);
    noiseModule.SetOctaveCount(3);

    for (int x = -1; x <= CHUNK_SIZE; x++) {
        float nx = (Position.x * CHUNK_SIZE + x) / CHUNK_ZOOM;
        bool x_in_chunk = (x >= 0 && x < CHUNK_SIZE);

        for (int y = -1; y <= CHUNK_SIZE; y++) {
            float ny = (Position.y * CHUNK_SIZE + y) / CHUNK_ZOOM;
            bool y_in_chunk = (y >= 0 && y < CHUNK_SIZE);

            for (int z = -1; z <= CHUNK_SIZE; z++) {
                float nz = (Position.z * CHUNK_SIZE + z) / CHUNK_ZOOM;
                bool z_in_chunk = (z >= 0 && z < CHUNK_SIZE);

                double noiseValue = noiseModule.GetValue(nx, ny, nz) - ny;

                if (noiseValue >= NOISE_DENSITY_BLOCK) {
                    if (x_in_chunk && y_in_chunk && z_in_chunk) {
                        BlockMap[x][y][z] = 1;
                        Blocks.push_back(glm::vec3(x, y, z));
                    }
                }
                else {
                    if (y_in_chunk && z_in_chunk) {
                        if (x > 0) {
                            SeesAir[x - 1][y][z] += pow(2, RIGHT);
                        }
                        if (x < CHUNK_SIZE - 1) {
                            SeesAir[x + 1][y][z] += pow(2, LEFT);
                        }
                    }

                    if (x_in_chunk && z_in_chunk) {
                        if (y > 0) {
                            SeesAir[x][y - 1][z] += pow(2, UP);
                        }
                        if (y < CHUNK_SIZE - 1) {
                            SeesAir[x][y + 1][z] += pow(2, DOWN);
                        }
                    }

                    if (x_in_chunk && y_in_chunk) {
                        if (z > 0) {
                            SeesAir[x][y][z - 1] += pow(2, FRONT);
                        }
                        if (z < CHUNK_SIZE - 1) {
                            SeesAir[x][y][z + 1] += pow(2, BACK);
                        }
                    }
                }
            }
        }
    }
}

void Chunk::Mesh() {
    std::vector<float> data;
    std::vector<glm::vec3>::iterator block = Blocks.begin();

    while (block != Blocks.end()) {
        unsigned char seesAir = SeesAir[(int)block->x][(int)block->y][(int)block->z];

        if (seesAir == 0) {
            block = Blocks.erase(block);
        }
        else {
            int bit = 0;

            while (bit < 6) {
                if (seesAir & 0x01) {
                    for (int j = bit * 6; j < bit * 6 + 6; j++) {
                        data.push_back(vertices[j][0] + Position.x * CHUNK_SIZE + block->x);
                        data.push_back(vertices[j][1] + Position.y * CHUNK_SIZE + block->y);
                        data.push_back(vertices[j][2] + Position.z * CHUNK_SIZE + block->z);

                        for (int k = 3; k < 8; k++) {
                            data.push_back(vertices[j][k]);
                        }
                    }
                }
                bit++;
                seesAir = seesAir >> 1;
            }
            ++block;
        }
    }
    vbo.Data(data);
}

void Chunk::Draw() {
    vbo.Draw();
}