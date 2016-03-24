#include "Chunk.h"

#include <noise/noise.h>

static const int CHUNK_ZOOM = 50;
static const float NOISE_DENSITY_BLOCK = 0.5f;

using glm::vec3;
using std::vector;

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

Chunk::Chunk(vec3 position) {
    Position = position;
}

int Chunk::GetBlock(vec3 position) {
    return BlockMap[(int) position.x][(int) position.y][(int) position.z];
}

void Chunk::SetBlock(vec3 position, char value) {
    BlockMap[(int) position.x][(int) position.y][(int) position.z] = value;
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
                        Blocks.insert(vec3(x, y, z));
                    }
                }
                else {
                    if (y_in_chunk && z_in_chunk) {
                        if (x > 0) {
                            SeesAir[x - 1][y][z] += pow(2, RIGHT);
                            Empty = false;
                        }
                        if (x < CHUNK_SIZE - 1) {
                            SeesAir[x + 1][y][z] += pow(2, LEFT);
                            Empty = false;
                        }
                    }

                    if (x_in_chunk && z_in_chunk) {
                        if (y > 0) {
                            SeesAir[x][y - 1][z] += pow(2, UP);
                            Empty = false;
                        }
                        if (y < CHUNK_SIZE - 1) {
                            SeesAir[x][y + 1][z] += pow(2, DOWN);
                            Empty = false;
                        }
                    }

                    if (x_in_chunk && y_in_chunk) {
                        if (z > 0) {
                            SeesAir[x][y][z - 1] += pow(2, FRONT);
                            Empty = false;
                        }
                        if (z < CHUNK_SIZE - 1) {
                            SeesAir[x][y][z + 1] += pow(2, BACK);
                            Empty = false;
                        }
                    }
                }
            }
        }
    }
}

void Chunk::Mesh() {
    if (Empty || Blocks.size() == 0) {
        return;
    }

    vector<float> data;
    std::set<vec3>::iterator block = Blocks.begin();

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

    if (data.size() > 0) {
        vbo.Data(data);
    }
}

void Chunk::RemoveBlock(vec3 position) {
    vector<vec3> meshingList;

    SetBlock(position, 0);
    Blocks.erase(position);

    vector<std::pair<vec3, vec3>> neighbors = Get_Neighbors(Position, position);

    for (int i = 0; i < 6; i++) {
        vec3 chunk = neighbors[i].first;
        vec3 tile = neighbors[i].second;

        if (ChunkMap.count(chunk)) {
            if (ChunkMap[chunk]->GetBlock(tile) > 0) {
                ChunkMap[chunk]->Blocks.insert(tile);
                ChunkMap[chunk]->SeesAir[(int) tile.x][(int) tile.y][(int) tile.z] += pow(2, i);

                meshingList.push_back(chunk);
            }
        }
    }

    Mesh();

    for (int i = 0; i < meshingList.size(); i++) {
        ChunkMap[meshingList[i]]->Empty = false;
        ChunkMap[meshingList[i]]->Mesh();
    }
}

vector<std::pair<vec3, vec3>> Get_Neighbors(vec3 chunk, vec3 tile) {
    vector<std::pair<vec3, vec3>> results;
    vec3 worldPos = Get_World_Pos(chunk, tile);

    vec3 neighborOffsets[6] = {
            vec3(1, 0, 0), vec3(-1, 0, 0),
            vec3(0, 1, 0), vec3(0, -1, 0),
            vec3(0, 0, 1), vec3(0, 0, -1)
    };

    for (int i = 0; i < 6; i++) {
        vector<vec3> neighbor = Get_Chunk_Pos(worldPos + neighborOffsets[i]);
        results.push_back(std::make_pair(neighbor[0], neighbor[1]));
    }

    return results;
}

vector<vec3> Get_Chunk_Pos(vec3 worldPos) {
    vec3 chunk(floor(worldPos.x / CHUNK_SIZE), floor(worldPos.y / CHUNK_SIZE), floor(worldPos.z / CHUNK_SIZE));
    vec3 tile(floor(worldPos.x - (chunk.x * CHUNK_SIZE)), floor(worldPos.y - (chunk.y * CHUNK_SIZE)), floor(worldPos.z - (chunk.z * CHUNK_SIZE)));

    return vector<vec3>{chunk, tile};
}

vec3 Get_World_Pos(vec3 chunk, vec3 tile) {
    chunk *= CHUNK_SIZE;
    return chunk + tile;
}