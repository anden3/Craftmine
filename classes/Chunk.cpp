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

int Chunk::GetBlock(glm::vec3 position) {
    return BlockMap[(int) position.x][(int) position.y][(int) position.z];
}

void Chunk::SetBlock(glm::vec3 position, char value) {
    BlockMap[int(position.x)][(int) position.y][(int) position.z] = value;
}

bool Chunk::Is_Empty() {
	noiseModule.SetPersistence(0.5);
	noiseModule.SetOctaveCount(3);

	int gx = int(Position.x) * CHUNK_SIZE;
	int gy = int(Position.y) * CHUNK_SIZE;
	int gz = int(Position.z) * CHUNK_SIZE;

	int x1 = gx / CHUNK_ZOOM;
	int x2 = (gx + (CHUNK_SIZE - 1)) / CHUNK_ZOOM;

	for (int y = 0; y < CHUNK_SIZE; y++) {
		int ny = (gy + y) / CHUNK_ZOOM;

		for (int z = 0; z < CHUNK_SIZE; z++) {
			int nz = (gz + z) / CHUNK_ZOOM;

			bool x1Block = noiseModule.GetValue(x1, ny, nz) - ny < NOISE_DENSITY_BLOCK;
			bool x2Block = noiseModule.GetValue(x2, ny, nz) - ny < NOISE_DENSITY_BLOCK;

			if (x1Block || x2Block) {
				return false;
			}
		}
	}

	int y1 = gy / CHUNK_ZOOM;
	int y2 = (gy + (CHUNK_SIZE - 1)) / CHUNK_ZOOM;

	for (int x = 0; x < CHUNK_SIZE; x++) {
		int nx = (gx + x) / CHUNK_ZOOM;

		for (int z = 0; z < CHUNK_SIZE; z++) {
			int nz = (gz + z) / CHUNK_ZOOM;

			bool y1Block = noiseModule.GetValue(nx, y1, nz) - y1 < NOISE_DENSITY_BLOCK;
			bool y2Block = noiseModule.GetValue(nx, y2, nz) - y2 < NOISE_DENSITY_BLOCK;

			if (y1Block || y2Block) {
				return false;
			}
		}
	}

	int z1 = gz / CHUNK_ZOOM;
	int z2 = (gz + (CHUNK_SIZE - 1)) / CHUNK_ZOOM;

	for (int x = 0; x < CHUNK_SIZE; x++) {
		int nx = (gx + x) / CHUNK_ZOOM;

		for (int y = 0; y < CHUNK_SIZE; y++) {
			int ny = (gy + y) / CHUNK_ZOOM;

			bool z1Block = noiseModule.GetValue(nx, ny, z1) - ny < NOISE_DENSITY_BLOCK;
			bool z2Block = noiseModule.GetValue(nx, ny, z2) - ny < NOISE_DENSITY_BLOCK;

			if (z1Block || z2Block) {
				return false;
			}
		}
	}

	return true;
}

void Chunk::UpdateAir(glm::ivec3 pos, glm::bvec3 inChunk) {
	bool chunkTests[3] = { inChunk.y && inChunk.z, inChunk.x && inChunk.z, inChunk.x && inChunk.y };
	int positions[3] = { pos.x, pos.y, pos.z };
	glm::ivec3 offsets[3] = { glm::ivec3(1, 0, 0), glm::ivec3(0, 1, 0), glm::ivec3(0, 0, 1) };

	for (int i = 0; i < 3; i++) {
		if (chunkTests[i]) {
			if (positions[i] < CHUNK_SIZE - 1) {
				glm::ivec3 index = pos + offsets[i];
				SeesAir[index.x][index.y][index.z] |= 1 << i * 2;
			}

			if (positions[i] > 0) {
				glm::ivec3 index = pos - offsets[i];
				SeesAir[index.x][index.y][index.z] |= 1 << (i * 2 + 1);
			}
		}
	}
}

void Chunk::Generate() {
	if (Is_Empty()) {
		EmptyChunks.insert(Position);
		return;
	}

    for (int x = -1; x <= CHUNK_SIZE; x++) {
        float nx = (Position.x * CHUNK_SIZE + x) / CHUNK_ZOOM;

        for (int y = -1; y <= CHUNK_SIZE; y++) {
            float ny = (Position.y * CHUNK_SIZE + y) / CHUNK_ZOOM;

            for (int z = -1; z <= CHUNK_SIZE; z++) {
                float nz = (Position.z * CHUNK_SIZE + z) / CHUNK_ZOOM;

				glm::bvec3 inChunk = glm::bvec3(
					x >= 0 && x < CHUNK_SIZE,
					y >= 0 && y < CHUNK_SIZE,
					z >= 0 && z < CHUNK_SIZE
				);

                double noiseValue = noiseModule.GetValue(nx, ny, nz) - ny * 2;

                if (noiseValue >= NOISE_DENSITY_BLOCK) {
                    if (inChunk.x && inChunk.y && inChunk.z) {
                        BlockMap[x][y][z] = 1;
                        Blocks.insert(glm::vec3(x, y, z));
                    }
                }
                else {
					UpdateAir(glm::ivec3(x, y, z), inChunk);
                }
            }
        }
    }
}

int Chunk::GetAO(glm::vec3 block, int face, int offset) {
	glm::ivec3 vertex(vertices[offset][0], vertices[offset][1], vertices[offset][2]);
	int ao = 3;

	glm::vec3 offsets[6][2][2][3] = {
		{ // Left
			{ { glm::vec3(-1, -1, 0), glm::vec3(-1, 0, -1), glm::vec3(-1, -1, -1) },
			{ glm::vec3(-1, 1, 0), glm::vec3(-1, 0, -1), glm::vec3(-1, 1, -1) } },

			{ { glm::vec3(-1, -1, 0), glm::vec3(-1, 0, 1), glm::vec3(-1, -1, 1) },
			{ glm::vec3(-1, 1, 0), glm::vec3(-1, 0, 1), glm::vec3(-1, 1, 1) } }
		},

		{ // Right
			{ { glm::vec3(1, -1, 0), glm::vec3(1, 0, -1), glm::vec3(1, -1, -1) },
			{ glm::vec3(1, 1, 0), glm::vec3(1, 0, -1), glm::vec3(1, 1, -1) } },

			{ { glm::vec3(1, -1, 0), glm::vec3(1, 0, 1), glm::vec3(1, -1, 1) },
			{ glm::vec3(1, 1, 0), glm::vec3(1, 0, 1), glm::vec3(1, 1, 1) } }
		},

		{ // Down
			{ { glm::vec3(-1, -1, 0), glm::vec3(0, -1, -1), glm::vec3(-1, -1, -1) },
			{ glm::vec3(-1, -1, 0), glm::vec3(0, -1, 1), glm::vec3(-1, -1, 1) } },

			{ { glm::vec3(1, -1, 0), glm::vec3(0, -1, -1), glm::vec3(1, -1, -1) },
			{ glm::vec3(1, -1, 0), glm::vec3(0, -1, 1), glm::vec3(1, -1, 1) } }

		},

		{ // Up
			{ { glm::vec3(-1, 1, 0), glm::vec3(0, 1, -1), glm::vec3(-1, 1, -1) },
			{ glm::vec3(-1, 1, 0), glm::vec3(0, 1, 1), glm::vec3(-1, 1, 1) } },

			{ { glm::vec3(1, 1, 0), glm::vec3(0, 1, -1), glm::vec3(1, 1, -1) },
			{ glm::vec3(1, 1, 0), glm::vec3(0, 1, 1), glm::vec3(1, 1, 1) } }
		},

		{ // Back
			{ { glm::vec3(0, -1, -1), glm::vec3(-1, 0, -1), glm::vec3(-1, -1, -1) },
			{ glm::vec3(0, 1, -1), glm::vec3(-1, 0, -1), glm::vec3(-1, 1, -1) } },

			{ { glm::vec3(0, -1, -1), glm::vec3(1, 0, -1), glm::vec3(1, -1, -1) },
			{ glm::vec3(0, 1, -1), glm::vec3(1, 0, -1), glm::vec3(1, 1, -1) } }

		},

		{ // Front
			{ { glm::vec3(0, -1, 1), glm::vec3(-1, 0, 1), glm::vec3(-1, -1, 1) },
			{ glm::vec3(0, 1, 1), glm::vec3(-1, 0, 1), glm::vec3(-1, 1, 1) }, },

			{ { glm::vec3(0, -1, 1), glm::vec3(1, 0, 1), glm::vec3(1, -1, 1) },
			{ glm::vec3(0, 1, 1), glm::vec3(1, 0, 1), glm::vec3(1, 1, 1) } }
		}
	};

	int vertexIndex[6][2] = {
		{vertex.z, vertex.y}, {vertex.z, vertex.y},
		{vertex.x, vertex.z}, {vertex.x, vertex.z},
		{vertex.x, vertex.y}, {vertex.x, vertex.y}
	};

	for (int i = 0; i < 3; i++) {
		if (Blocks.find(block + offsets[face][vertexIndex[face][0]][vertexIndex[face][1]][i]) != Blocks.end()) {
			if (ao == 2 && i == 1) return 0;
			ao--;
		}
	}
	
	return ao;
}

void Chunk::Mesh() {
    std::vector<float> data;
    std::set<glm::vec3>::iterator block = Blocks.begin();

    while (block != Blocks.end()) {
        unsigned char seesAir = SeesAir[(int)block->x][(int)block->y][(int)block->z];

        if (seesAir == 0) {
            block = Blocks.erase(block);
        }
        else {
            int bit = 0;

            while (bit < 6) {
                if (seesAir & 1) {
                    for (int j = bit * 6; j < bit * 6 + 6; j++) {
                        data.push_back(vertices[j][0] + Position.x * CHUNK_SIZE + block->x);
                        data.push_back(vertices[j][1] + Position.y * CHUNK_SIZE + block->y);
                        data.push_back(vertices[j][2] + Position.z * CHUNK_SIZE + block->z);

                        for (int k = 3; k < 8; k++) {
                            data.push_back(vertices[j][k]);
                        }

						data.push_back(float(GetAO(*block, bit, j)));
                    }
                }
                bit++;
                seesAir = seesAir >> 1;
            }
            ++block;
        }
    }

    if (data.size() > 0) {
		DataQueue[Position] = data;
    }
}

void Chunk::RemoveBlock(glm::vec3 position) {
    SetBlock(position, 0);
    Blocks.erase(position);

	std::vector<glm::vec3> meshingList;
    std::vector<std::pair<glm::vec3, glm::vec3>> neighbors = Get_Neighbors(Position, position);

    for (int i = 0; i < 6; i++) {
        glm::vec3 chunk = neighbors[i].first;
        glm::vec3 tile = neighbors[i].second;

		if (ChunkMap.count(chunk)) {
            if (ChunkMap[chunk]->GetBlock(tile)) {
                ChunkMap[chunk]->Blocks.insert(tile);
				ChunkMap[chunk]->SeesAir[(int)tile.x][(int)tile.y][(int)tile.z] |= 1 << i;

                if (chunk != Position) meshingList.push_back(chunk);
            }
        }
    }

    Mesh();

    for (unsigned int i = 0; i < meshingList.size(); i++) {
        ChunkMap[meshingList[i]]->Mesh();
    }
}

void Chunk::AddBlock(glm::vec3 position, glm::vec3 diff) {
	SetBlock(position, 1);
	Blocks.insert(position);

	std::vector<glm::vec3> meshingList;
	std::vector<std::pair<glm::vec3, glm::vec3>> neighbors = Get_Neighbors(Position, position);

	for (int i = 0; i < 6; i++) {
		glm::vec3 chunk = neighbors[i].first;
		glm::vec3 tile = neighbors[i].second;

		if (ChunkMap.count(chunk)) {
			if (ChunkMap[chunk]->GetBlock(tile)) {
				ChunkMap[chunk]->SeesAir[(int)tile.x][(int)tile.y][(int)tile.z] &= ~(1 << i);

				if (chunk != Position) meshingList.push_back(chunk);
			}
			else {
				int direction = i + 1;

				if (i % 2 != 0) {
					direction = i - 1;
				}

				SeesAir[(int)position.x][(int)position.y][(int)position.z] |= 1 << direction;
			}
		}
	}
	
	Mesh();
	

	for (unsigned int i = 0; i < meshingList.size(); i++) {
		ChunkMap[meshingList[i]]->Mesh();
	}
}

std::vector<std::pair<glm::vec3, glm::vec3>> Get_Neighbors(glm::vec3 chunk, glm::vec3 tile) {
    std::vector<std::pair<glm::vec3, glm::vec3>> results;
    glm::vec3 worldPos = Get_World_Pos(chunk, tile);

    glm::vec3 neighborOffsets[6] = {
            glm::vec3(1, 0, 0), glm::vec3(-1, 0, 0),
            glm::vec3(0, 1, 0), glm::vec3(0, -1, 0),
            glm::vec3(0, 0, 1), glm::vec3(0, 0, -1)
    };

    for (int i = 0; i < 6; i++) {
        std::vector<glm::vec3> neighbor = Get_Chunk_Pos(worldPos + neighborOffsets[i]);
        results.push_back(std::make_pair(neighbor[0], neighbor[1]));
    }

    return results;
}

std::vector<glm::vec3> Get_Chunk_Pos(glm::vec3 worldPos) {
    glm::vec3 chunk(floor(worldPos.x / CHUNK_SIZE), floor(worldPos.y / CHUNK_SIZE), floor(worldPos.z / CHUNK_SIZE));
    glm::vec3 tile(floor(worldPos.x - (chunk.x * CHUNK_SIZE)), floor(worldPos.y - (chunk.y * CHUNK_SIZE)), floor(worldPos.z - (chunk.z * CHUNK_SIZE)));

    return std::vector<glm::vec3>{chunk, tile};
}

glm::vec3 Get_World_Pos(glm::vec3 chunk, glm::vec3 tile) {
    chunk *= CHUNK_SIZE;
    return chunk + tile;
}