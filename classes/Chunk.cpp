#include "Chunk.h"

#include <random>

#include <noise/noise.h>

const int CHUNK_ZOOM = 50;
const float NOISE_DENSITY_BLOCK = 0.5f;

bool Seeded = false;

noise::module::Perlin noiseModule;

enum Directions {
    LEFT,
    RIGHT,
    DOWN,
    UP,
    BACK,
    FRONT
};

std::map<unsigned char, glm::vec2> textureCoords = {
	{1, glm::vec2(2, 1)}, // Stone

	{3, glm::vec2(3, 1)}, // Dirt
	{4, glm::vec2(1, 2)}, // Cobblestone
	{5, glm::vec2(1, 5)}, // Wooden Planks

	{7, glm::vec2(2, 2)}, // Bedrock
	
	{9, glm::vec2(13, 14)}, // Water
	{11, glm::vec2(15, 14)}, // Lava

	{12, glm::vec2(2, 3)}, // Sand
	{13, glm::vec2(2, 4)}, // Gravel

	{14, glm::vec2(3, 1)}, // Gold Ore
	{15, glm::vec2(3, 2)}, // Iron Ore
	{16, glm::vec2(3, 3)}, // Coal Ore

	{17, glm::vec2(5, 4)}, // Transparent Leaves
};

std::vector<glm::vec2> grassTextures = { glm::vec2(4, 1), glm::vec2(4, 1), glm::vec2(3, 1), glm::vec2(1, 1), glm::vec2(4, 1), glm::vec2(4, 1) }; // ID 2
std::vector<glm::vec2> logTextures = { glm::vec2(5, 2), glm::vec2(5, 2), glm::vec2(6, 2), glm::vec2(6, 2), glm::vec2(5, 2), glm::vec2(5, 2) }; // ID 17

std::map<glm::vec2, std::set<glm::vec2, Vec2Comparator>, Vec2Comparator> topBlocks;
std::map<glm::vec3, std::set<LightNode, LightNodeComparator>, Vec3Comparator> UnloadedLightQueue;

float vertices[6][6][3] = {
		{ {0, 0, 0}, {0, 1, 1}, {0, 1, 0}, {0, 1, 1}, {0, 0, 0}, {0, 0, 1} },
		{ {1, 0, 0}, {1, 1, 0}, {1, 1, 1}, {1, 0, 0}, {1, 1, 1}, {1, 0, 1} },
		{ {0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {1, 0, 1}, {0, 0, 1}, {0, 0, 0} },
		{ {0, 1, 0}, {1, 1, 1}, {1, 1, 0}, {0, 1, 0}, {0, 1, 1}, {1, 1, 1} },
		{ {0, 0, 0}, {1, 1, 0}, {1, 0, 0}, {0, 0, 0}, {0, 1, 0}, {1, 1, 0} },
		{ {0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {1, 1, 1}, {0, 1, 1}, {0, 0, 1} }
};

float normals[6][3] = { {-1, 0, 0}, {1, 0, 0}, {0, -1, 0}, {0, 1, 0}, {0, 0, -1}, {0, 0, 1} };

float tex_coords[6][6][2] = {
	{ {0, 1}, {1, 0}, {0, 0}, {1, 0}, {0, 1}, {1, 1} },
	{ {1, 1}, {1, 0}, {0, 0}, {1, 1}, {0, 0}, {0, 1} },
	{ {0, 0}, {1, 0}, {1, 1}, {1, 1}, {0, 1}, {0, 0} },
	{ {0, 0}, {1, 1}, {1, 0}, {0, 0}, {0, 1}, {1, 1} },
	{ {1, 1}, {0, 0}, {0, 1}, {1, 1}, {1, 0}, {0, 0} },
	{ {0, 1}, {1, 1}, {1, 0}, {1, 0}, {0, 0}, {0, 1} }
};

void Seed() {
    Seeded = true;
    
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> uni(0, 10000);
    
    noiseModule.SetSeed(uni(rng));
}

Chunk::Chunk(glm::vec3 position) {
    if (!Seeded) {
        Seed();
    }
    
    Position = position;
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
    
    glm::vec2 topPos(Position.x, Position.z);
    
    if (Position.y == 3) {
        topBlocks[topPos].clear();
    }

    for (int x = -1; x <= CHUNK_SIZE; x++) {
        float nx = (Position.x * CHUNK_SIZE + x) / CHUNK_ZOOM;

        for (int z = -1; z <= CHUNK_SIZE; z++) {
            float nz = (Position.z * CHUNK_SIZE + z) / CHUNK_ZOOM;
            
            for (int y = CHUNK_SIZE; y >= -1; y--) {
                float ny = (Position.y * CHUNK_SIZE + y) / CHUNK_ZOOM;

				glm::bvec3 inChunk = glm::bvec3(
					x >= 0 && x < CHUNK_SIZE,
					y >= 0 && y < CHUNK_SIZE,
					z >= 0 && z < CHUNK_SIZE
				);

                double noiseValue = noiseModule.GetValue(nx, ny, nz) - ny * 2;
                
                glm::ivec3 block(x, y, z);

                if (noiseValue >= NOISE_DENSITY_BLOCK) {
                    if (inChunk.x && inChunk.y && inChunk.z) {
                        glm::vec2 topBlock(x, z);
                        
                        if (!topBlocks[topPos].count(topBlock)) {
                            topBlocks[topPos].insert(topBlock);
                            TopBlocks.insert(block);
                            
                            Set_Light(block, SUN_LIGHT_LEVEL);
                            LightQueue.emplace(Position, block, SUN_LIGHT_LEVEL);
                        }
                        
                        BlockMap[x][y][z] = 2;
                        Blocks.insert(block);
                    }
                }
                else {
					UpdateAir(block, inChunk);
                }
            }
        }
    }
    
    Generated = true;
}

void Chunk::Light() {
    if (UnloadedLightQueue.count(Position)) {
        for (auto const &node : UnloadedLightQueue[Position]) {
            if (!Get_Block(node.Tile)) continue;
            if (!Get_Air(node.Tile) && node.Underground) continue;
            if (Get_Top(node.Tile)) continue;
            if (Get_Light(node.Tile) + 2 >= node.LightLevel + 1) continue;
            
            Set_Light(node.Tile, node.LightLevel);
            LightQueue.push(node);
        }
        
        UnloadedLightQueue.erase(Position);
    }
    
    while (!LightQueue.empty()) {
        LightNode node = LightQueue.front();
        LightQueue.pop();
        glm::vec3 tile = node.Tile;
        
        int lightLevel = Get_Light(tile);
        bool underground = !Get_Air(tile);
        
        std::vector<std::pair<glm::vec3, glm::vec3>> neighbors = Get_Neighbors(Position, tile);
        
        for (auto const &neighbor : neighbors) {
            if (neighbor.first != Position) {
                if (!Exists(neighbor.first)) {
                    UnloadedLightQueue[neighbor.first].emplace(neighbor.first, neighbor.second, lightLevel - 1, underground);
                    continue;
                }
                
                Chunk* neighborChunk = ChunkMap[neighbor.first];
                
                if (!neighborChunk->Get_Block(neighbor.second)) continue;
                if (!neighborChunk->Get_Air(neighbor.second) && underground) continue;
                if (neighborChunk->Get_Top(neighbor.second)) continue;
                if (neighborChunk->Get_Light(neighbor.second) + 2 >= lightLevel) continue;
                
                neighborChunk->Set_Light(neighbor.second, lightLevel - 1);
                
                neighborChunk->LightQueue.emplace(neighbor.first, neighbor.second);
                neighborChunk->Meshed = false;
            }
            else {
                if (!Get_Block(neighbor.second)) continue;
                if (!Get_Air(neighbor.second) && underground) continue;
                if (Get_Top(neighbor.second)) continue;
                if (Get_Light(neighbor.second) + 2 >= lightLevel) continue;
                
                Set_Light(neighbor.second, lightLevel - 1);
                
                LightQueue.emplace(neighbor.first, neighbor.second);
            }
        }
    }
}

int Chunk::GetAO(glm::vec3 block, int face, int index) {
	glm::ivec3 vertex(vertices[face][index][0], vertices[face][index][1], vertices[face][index][2]);
	int ao = 0;

	glm::vec3 offsets[6][2][2][3] = {
		  { { { glm::vec3(-1, -1,  0), glm::vec3(-1,  0, -1), glm::vec3(-1, -1, -1) },
			  { glm::vec3(-1,  1,  0), glm::vec3(-1,  0, -1), glm::vec3(-1,  1, -1) } },

			{ { glm::vec3(-1, -1,  0), glm::vec3(-1,  0,  1), glm::vec3(-1, -1,  1) },
			  { glm::vec3(-1,  1,  0), glm::vec3(-1,  0,  1), glm::vec3(-1,  1,  1) } } },

		  { { { glm::vec3( 1, -1,  0), glm::vec3( 1,  0, -1), glm::vec3( 1, -1, -1) },
			  { glm::vec3( 1,  1,  0), glm::vec3( 1,  0, -1), glm::vec3( 1,  1, -1) } },

			{ { glm::vec3( 1, -1,  0), glm::vec3( 1,  0,  1), glm::vec3( 1, -1,  1) },
			  { glm::vec3( 1,  1,  0), glm::vec3( 1,  0,  1), glm::vec3( 1,  1,  1) } } },

		  { { { glm::vec3(-1, -1,  0), glm::vec3( 0, -1, -1), glm::vec3(-1, -1, -1) },
			  { glm::vec3(-1, -1,  0), glm::vec3( 0, -1,  1), glm::vec3(-1, -1,  1) } },

			{ { glm::vec3( 1, -1,  0), glm::vec3( 0, -1, -1), glm::vec3( 1, -1, -1) },
			  { glm::vec3( 1, -1,  0), glm::vec3( 0, -1,  1), glm::vec3( 1, -1,  1) } } },

		  { { { glm::vec3(-1,  1,  0), glm::vec3( 0,  1, -1), glm::vec3(-1,  1, -1) },
			  { glm::vec3(-1,  1,  0), glm::vec3( 0,  1,  1), glm::vec3(-1,  1,  1) } },

			{ { glm::vec3( 1,  1,  0), glm::vec3( 0,  1, -1), glm::vec3( 1,  1, -1) },
			  { glm::vec3( 1,  1,  0), glm::vec3( 0,  1,  1), glm::vec3( 1,  1,  1) } } },

		  { { { glm::vec3( 0, -1, -1), glm::vec3(-1,  0, -1), glm::vec3(-1, -1, -1) },
			  { glm::vec3( 0,  1, -1), glm::vec3(-1,  0, -1), glm::vec3(-1,  1, -1) } },

			{ { glm::vec3( 0, -1, -1), glm::vec3( 1,  0, -1), glm::vec3( 1, -1, -1) },
			  { glm::vec3( 0,  1, -1), glm::vec3( 1,  0, -1), glm::vec3( 1,  1, -1) } } },

		  { { { glm::vec3( 0, -1,  1), glm::vec3(-1,  0,  1), glm::vec3(-1, -1,  1) },
			  { glm::vec3( 0,  1,  1), glm::vec3(-1,  0,  1), glm::vec3(-1,  1,  1) } },

			{ { glm::vec3( 0, -1,  1), glm::vec3( 1,  0,  1), glm::vec3( 1, -1,  1) },
			  { glm::vec3( 0,  1,  1), glm::vec3( 1,  0,  1), glm::vec3( 1,  1,  1) } } }
	};

	int vertexIndex[6][2] = {
		{vertex.z, vertex.y}, {vertex.z, vertex.y},
		{vertex.x, vertex.z}, {vertex.x, vertex.z},
		{vertex.x, vertex.y}, {vertex.x, vertex.y}
	};

	for (int i = 0; i < 3; i++) {
		if (Blocks.count(block + offsets[face][vertexIndex[face][0]][vertexIndex[face][1]][i])) {
			if (ao == 1 && i == 1) return 0;
			ao++;
		}
	}
	
	return ao;
}

bool Chunk::Check_Grass(glm::vec3 pos) {
	if (pos.y == CHUNK_SIZE - 1) {
		glm::vec3 nextChunk = glm::vec3(Position.x, Position.y + 1, Position.z);

		if (Exists(nextChunk)) {
			return !ChunkMap[nextChunk]->Get_Block(glm::vec3(pos.x, 0, pos.z));
		}
        
        double nx = (Position.x * CHUNK_SIZE + pos.x) / CHUNK_ZOOM;
        double ny = (Position.y * CHUNK_SIZE + pos.y + 1) / CHUNK_ZOOM;
        double nz = (Position.z * CHUNK_SIZE + pos.z) / CHUNK_ZOOM;
        
        return noiseModule.GetValue(nx, ny, nz) - ny * 2 < NOISE_DENSITY_BLOCK;
	}
    
	return !Get_Block(glm::vec3(pos.x, pos.y + 1, pos.z));
}

void Chunk::Mesh() {    
    VBOData.clear();
    
    std::set<glm::vec3>::iterator block = Blocks.begin();

	float textureStep = (1.0f / 16.0f);

    while (block != Blocks.end()) {
        unsigned char seesAir = SeesAir[int(block->x)][int(block->y)][int(block->z)];
        float lightValue = float(Get_Light(*block));

        if (seesAir == 0) {
            block = Blocks.erase(block);
        }
        else {
            int bit = 0;
			unsigned char blockType = Get_Block(*block);

			if (blockType == 2) {
				bool isGrass = Check_Grass(*block);

				if (!isGrass) {
					blockType = 3;
					Set_Block(*block, 3);
				}
			}
			glm::vec2 texPosition = textureCoords[blockType];

			float texStartX = textureStep * (texPosition.x - 1.0f);
			float texStartY = textureStep * (texPosition.y - 1.0f);

            while (bit < 6) {
                if (seesAir & 1) {
                    for (int j = 0; j < 6; j++) {
                        VBOData.push_back(vertices[bit][j][0] + Position.x * CHUNK_SIZE + block->x);
                        VBOData.push_back(vertices[bit][j][1] + Position.y * CHUNK_SIZE + block->y);
                        VBOData.push_back(vertices[bit][j][2] + Position.z * CHUNK_SIZE + block->z);

						VBOData.push_back(normals[bit][0]);
						VBOData.push_back(normals[bit][1]);
						VBOData.push_back(normals[bit][2]);

						if (blockType == 2) {
							VBOData.push_back(textureStep * (grassTextures[bit].x - 1.0f) + tex_coords[bit][j][0] * textureStep);
							VBOData.push_back(textureStep * (grassTextures[bit].y - 1.0f) + tex_coords[bit][j][1] * textureStep);
						}

						else {
							VBOData.push_back(texStartX + tex_coords[bit][j][0] * textureStep);
							VBOData.push_back(texStartY + tex_coords[bit][j][1] * textureStep);
						}
                        
                        VBOData.push_back(lightValue);
						VBOData.push_back(float(GetAO(*block, bit, j)));
                    }
                }
                bit++;
                seesAir = seesAir >> 1;
            }
            ++block;
        }
    }

    if (VBOData.size() > 0) {
        if (!Meshed) {
            Meshed = true;
        }
        else {
            vbo.Data(VBOData);
        }
    }
}

void Chunk::Remove_Block(glm::vec3 position) {
    Set_Block(position, 0);
    Blocks.erase(position);

	std::vector<glm::vec3> meshingList;
    std::vector<std::pair<glm::vec3, glm::vec3>> neighbors = Get_Neighbors(Position, position);

    for (int i = 0; i < 6; i++) {
        glm::vec3 chunk = neighbors[i].first;
        glm::vec3 tile = neighbors[i].second;
        
        if (chunk != Position) {
            if (Exists(chunk)) {
                if (ChunkMap[chunk]->Get_Block(tile)) {
                    ChunkMap[chunk]->Blocks.insert(tile);
                    ChunkMap[chunk]->SeesAir[(int)tile.x][(int)tile.y][(int)tile.z] |= 1 << i;
                    
                    meshingList.push_back(chunk);
                }
            }
        }
        else {
            if (Get_Block(tile)) {
                Blocks.insert(tile);
                SeesAir[(int)tile.x][(int)tile.y][(int)tile.z] |= 1 << i;
            }
        }
    }
    
    Light();
    Mesh();
    
    for (auto const &chunk : meshingList) {
        ChunkMap[chunk]->Light();
        ChunkMap[chunk]->Mesh();
    }
}

void Chunk::Add_Block(glm::vec3 position, glm::vec3 diff, int blockType) {
	Set_Block(position, blockType);
	Blocks.insert(position);

	std::vector<glm::vec3> meshingList;
	std::vector<std::pair<glm::vec3, glm::vec3>> neighbors = Get_Neighbors(Position, position);

	for (int i = 0; i < 6; i++) {
		glm::vec3 chunk = neighbors[i].first;
		glm::vec3 tile = neighbors[i].second;

		if (ChunkMap.count(chunk)) {
			if (ChunkMap[chunk]->Get_Block(tile)) {
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
	
    Light();
	Mesh();

    for (auto const &chunk : meshingList) {
        ChunkMap[chunk]->Light();
        ChunkMap[chunk]->Mesh();
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

bool Is_Block(glm::vec3 pos) {
    std::vector<glm::vec3> chunkPos = Get_Chunk_Pos(pos);
    bool isBlock = false;
    
    if (Exists(chunkPos[0])) {
        isBlock = ChunkMap[chunkPos[0]]->Get_Block(chunkPos[1]) > 0;
    }
    
    return isBlock;
}