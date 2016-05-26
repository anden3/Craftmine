#include "Chunk.h"

#include <random>
#include <noise/noise.h>

const int CHUNK_ZOOM = 50;
const float NOISE_DENSITY_BLOCK = 0.5f;
const float NOISE_DENSITY_CAVE = -0.85f;

bool Seeded = false;

noise::module::Perlin noiseModule;
noise::module::RidgedMulti ridgedNoise;

enum Directions {LEFT, RIGHT, DOWN, UP, BACK, FRONT};

std::map<glm::vec2, std::map<glm::vec2, int, Vec2Comparator>, Vec2Comparator> topBlocks;
std::map<glm::vec3, std::set<LightNode, LightNodeComparator>, Vec3Comparator> UnloadedLightQueue;

std::map<glm::vec3, std::map<glm::vec3, int, Vec3Comparator>, Vec3Comparator> ChangedBlocks;

void Seed() {
    Seeded = true;
    
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> uni(0, 10000);
    
    int seed = uni(rng);
    
    noiseModule.SetSeed(seed);
    noiseModule.SetPersistence(0.5);
    noiseModule.SetOctaveCount(3);
    
    ridgedNoise.SetSeed(seed);
    ridgedNoise.SetOctaveCount(2);
    ridgedNoise.SetFrequency(5.0);
}

Chunk::Chunk(glm::vec3 position) {
    if (!Seeded) {
        Seed();
    }
    
    Position = position;
}

void Chunk::UpdateAir(glm::ivec3 pos, glm::bvec3 inChunk) {
	bool chunkTests[3] = { inChunk.y && inChunk.z, inChunk.x && inChunk.z, inChunk.x && inChunk.y };
	glm::ivec3 offsets[3] = { glm::ivec3(1, 0, 0), glm::ivec3(0, 1, 0), glm::ivec3(0, 0, 1) };

	for (int i = 0; i < 3; i++) {
		if (chunkTests[i]) {
			if (pos[i] < CHUNK_SIZE - 1) {
				glm::ivec3 index = pos + offsets[i];
				SeesAir[index.x][index.y][index.z] |= 1 << i * 2;
			}

			if (pos[i] > 0) {
				glm::ivec3 index = pos - offsets[i];
				SeesAir[index.x][index.y][index.z] |= 1 << (i * 2 + 1);
			}
		}
	}
}

void Chunk::Generate() {
    glm::vec2 topPos = Position.xz();
    
    if (Position.y == 3) {
        topBlocks[topPos].clear();
    }

    for (int x = -1; x <= CHUNK_SIZE; x++) {
        float nx = (Position.x * CHUNK_SIZE + x) / CHUNK_ZOOM;

        for (int z = -1; z <= CHUNK_SIZE; z++) {
            float nz = (Position.z * CHUNK_SIZE + z) / CHUNK_ZOOM;

            for (int y = CHUNK_SIZE; y >= -1; y--) {
                float ny = (Position.y * CHUNK_SIZE + y) / CHUNK_ZOOM;
                
                glm::ivec3 block(x, y, z);

				glm::bvec3 inChunk = glm::bvec3(
					x >= 0 && x < CHUNK_SIZE,
					y >= 0 && y < CHUNK_SIZE,
					z >= 0 && z < CHUNK_SIZE
				);
                
                if (ChangedBlocks.count(Position) && ChangedBlocks[Position].count(block)) {
                    int type = ChangedBlocks[Position][block];
                    
                    if (type > 0) {
                        if (!topBlocks[topPos].count(block.xz())) {
                            topBlocks[topPos][block.xz()] = Position.y * CHUNK_SIZE + y;
                            TopBlocks.insert(block);
                            
                            Set_Light(block, SUN_LIGHT_LEVEL);
                            LightQueue.emplace(Position, block, SUN_LIGHT_LEVEL);
                        }
                        
                        Set_Block(block, type);
                        Blocks.insert(block);
                    }
                    else {
                        UpdateAir(block, inChunk);
                    }
                }
                else {
                    double noiseValue;
                    float densityThreshold;
                    
                    if (Position.y >= -3) {
                        noiseValue = noiseModule.GetValue(nx, ny, nz) - ny * 2;
                        densityThreshold = NOISE_DENSITY_BLOCK;
                    }
                    else {
                        noiseValue = ridgedNoise.GetValue(nx, ny, nz);
                        densityThreshold = NOISE_DENSITY_CAVE;
                    }
                    
                    if (noiseValue >= densityThreshold) {
                        if (inChunk.x && inChunk.y && inChunk.z) {
                            if (!topBlocks[topPos].count(block.xz())) {
                                topBlocks[topPos][block.xz()] = Position.y * CHUNK_SIZE + y;
                                TopBlocks.insert(block);
                                
                                Set_Light(block, SUN_LIGHT_LEVEL);
                                LightQueue.emplace(Position, block);
                                
                                Set_Block(block, 2);
                            }
                            else {
                                int depth = std::abs(topBlocks[topPos][block.xz()] - int(Position.y * CHUNK_SIZE + y));
                                
                                if (depth > 3) {
                                    Set_Block(block, 1);
                                }
                                else {
                                    Set_Block(block, 3);
                                }
                            }
                            
                            Blocks.insert(block);
                        }
                    }
                    else {
                        UpdateAir(block, inChunk);
                    }
                }
            }
        }
    }
    
    Generated = true;
}

bool Check_If_Node(glm::vec3 chunk, glm::vec3 tile, char lightLevel, bool underground, bool down) {
    Chunk* c = ChunkMap[chunk];
    
    if (c->Get_Block(tile)) {
        if (c->Get_Light(tile) + 2 < lightLevel) {
            if (c->Get_Air(tile) || !underground) {
                c->Set_Light(tile, lightLevel - (!down));
                return true;
            }
        }
    }
    return false;
}

void Chunk::Light(bool flag) {
    if (UnloadedLightQueue.count(Position)) {
        for (auto node : UnloadedLightQueue[Position]) {
            if (Check_If_Node(Position, node.Tile, node.LightLevel + 1, node.Underground, node.Down)) {
                LightQueue.push(node);
            }
        }
        
        UnloadedLightQueue.erase(Position);
    }
    
    while (!LightRemovalQueue.empty()) {
        LightNode node = LightRemovalQueue.front();
        LightRemovalQueue.pop();
        
        glm::vec3 chunk = node.Chunk;
        glm::vec3 tile = node.Tile;
        short lightLevel = node.LightLevel;
        
        bool underground = !ChunkMap[chunk]->Get_Air(tile);
        
        std::vector<std::pair<glm::vec3, glm::vec3>> neighbors = Get_Neighbors(chunk, tile);
        
        for (auto const &neighbor : neighbors) {
            if (Exists(neighbor.first)) {
                Chunk* neighborChunk = ChunkMap[neighbor.first];
                
                if (!neighborChunk->Get_Block(neighbor.second)) continue;
                if (!neighborChunk->Get_Air(neighbor.second) && underground) continue;
                
                int neighborLight = neighborChunk->Get_Light(neighbor.second);
                
                if (neighborLight != 0 || lightLevel == 0) {
                    neighborChunk->Set_Light(neighbor.second, (neighborLight > lightLevel || neighborLight == 0) ? neighborLight : 0);
                    
                    if (neighborLight != 0 && neighborLight < lightLevel) {
                        neighborChunk->LightRemovalQueue.emplace(neighbor.first, neighbor.second, neighborLight);
                    }
                    else {
                        neighborChunk->LightQueue.emplace(neighbor.first, neighbor.second);
                    }
                    
                    neighborChunk->Meshed = false;
                }
            }
        }
    }
    
    while (!LightQueue.empty()) {
        LightNode node = LightQueue.front();
        LightQueue.pop();
        
        glm::vec3 tile = node.Tile;
        std::vector<std::pair<glm::vec3, glm::vec3>> neighbors = Get_Neighbors(Position, tile);
        
        bool underground = !Get_Air(tile);
        char lightLevel = Get_Light(tile);
        
        int index = 0;
        
        for (auto const &neighbor : neighbors) {
            if (!ChunkMap.count(neighbor.first)) {
                UnloadedLightQueue[neighbor.first].emplace(neighbor.first, neighbor.second, lightLevel, index == UP, underground);
                continue;
            }
            else if (Check_If_Node(neighbor.first, neighbor.second, lightLevel, underground, index == UP)) {
                ChunkMap[neighbor.first]->LightQueue.emplace(neighbor.first, neighbor.second);
                
                if (neighbor.first != Position && flag) {
                    ChunkMap[neighbor.first]->Meshed = false;
                }
            }
        }
        
        ++index;
    }
}

int Chunk::GetAO(glm::vec3 block, int face, int index) {
	int ao = 0;

	static const glm::vec3 offsets[6][2][2][3] = {
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

	int vertexIndex[3][2] = { {int(vertices[face][index][2]), int(vertices[face][index][1])}, {int(vertices[face][index][0]), int(vertices[face][index][2])}, {int(vertices[face][index][0]), int(vertices[face][index][1])} };

	for (int i = 0; i < 3; i++) {
		if (Blocks.count(block + offsets[face][vertexIndex[face / 2][0]][vertexIndex[face / 2][1]][i])) {
            if (ao == 1 && i == 1) {
                return 0;
            }
            
			++ao;
		}
	}
	
	return ao;
}

void Chunk::Mesh() {
    VBOData.clear();
    
    std::set<glm::vec3>::iterator block = Blocks.begin();

    while (block != Blocks.end()) {
        unsigned char seesAir = SeesAir[int(block->x)][int(block->y)][int(block->z)];
        float lightValue = float(Get_Light(*block));

        if (seesAir == 0) {
            block = Blocks.erase(block);
        }
        else {
            int bit = 0;
			unsigned int blockType = Get_Block(*block);
			glm::vec2 texPosition = textureCoords[blockType];

			float texStartX = texPosition.x - 1.0f;
			float texStartY = texPosition.y - 1.0f;

            while (bit < 6) {
                if (seesAir & 1) {
                    for (int j = 0; j < 6; j++) {
                        if (CustomVertices.count(blockType)) {
                            VBOData.push_back(CustomVertices[blockType][bit][vertices[bit][j][0]].x + block->x + Position.x * CHUNK_SIZE);
                            VBOData.push_back(CustomVertices[blockType][bit][vertices[bit][j][1]].y + block->y + Position.y * CHUNK_SIZE);
                            VBOData.push_back(CustomVertices[blockType][bit][vertices[bit][j][2]].z + block->z + Position.z * CHUNK_SIZE);
                        }
                        else {
                            VBOData.push_back(vertices[bit][j][0] + block->x + Position.x * CHUNK_SIZE);
                            VBOData.push_back(vertices[bit][j][1] + block->y + Position.y * CHUNK_SIZE);
                            VBOData.push_back(vertices[bit][j][2] + block->z + Position.z * CHUNK_SIZE);
                        }
                        
                        if (CustomTexCoords.count(blockType)) {
                            VBOData.push_back(CustomTexCoords[blockType][bit][tex_coords[bit][j][0]].x / IMAGE_SIZE_X);
                            VBOData.push_back(CustomTexCoords[blockType][bit][tex_coords[bit][j][1]].y / IMAGE_SIZE_Y);
                        }

						else if (MultiTextures.count(blockType)) {
							VBOData.push_back((MultiTextures[blockType][bit].x - 1.0f + tex_coords[bit][j][0]) / IMAGE_SIZE_X);
							VBOData.push_back((MultiTextures[blockType][bit].y - 1.0f + tex_coords[bit][j][1]) / IMAGE_SIZE_Y);
						}

						else {
							VBOData.push_back((texStartX + tex_coords[bit][j][0]) / IMAGE_SIZE_X);
							VBOData.push_back((texStartY + tex_coords[bit][j][1]) / IMAGE_SIZE_Y);
						}
                        
                        VBOData.push_back(lightValue);
						VBOData.push_back(float(GetAO(*block, bit, j)));
                    }
                }
                ++bit;
                seesAir = seesAir >> 1;
            }
            ++block;
        }
    }

    if (VBOData.size() > 0) {
        Meshed = true;
    }
    
    DataUploaded = false;
}

void Chunk::Remove_Block(glm::ivec3 position) {
    Set_Block(position, 0);
    Blocks.erase(position);
    
    if (ChangedBlocks[Position].count(position)) {
        ChangedBlocks[Position].erase(position);
    }
    else {
        ChangedBlocks[Position][position] = 0;
    }
    
    bool lightBlocks = false;
    
    if (topBlocks[Position.xz()][position.xz()] == Position.y * CHUNK_SIZE + position.y) {
        lightBlocks = true;
        topBlocks[Position.xz()][position.xz()]--;
    }

	std::vector<Chunk*> meshingList;
    std::vector<std::pair<glm::vec3, glm::vec3>> neighbors = Get_Neighbors(Position, position);

    for (int i = 0; i < 6; i++) {
        glm::vec3 chunk = neighbors[i].first;
        glm::uvec3 tile = neighbors[i].second;
        
        if (chunk != Position) {
            if (Exists(chunk)) {
                if (ChunkMap[chunk]->Get_Block(tile)) {
                    ChunkMap[chunk]->Blocks.insert(tile);
                    ChunkMap[chunk]->SeesAir[tile.x][tile.y][tile.z] |= 1 << i;
                    
                    if (lightBlocks) {
                        ChunkMap[chunk]->Set_Light(position, SUN_LIGHT_LEVEL);
                    }
                    
                    ChunkMap[chunk]->LightQueue.emplace(chunk, position);
                    meshingList.push_back(ChunkMap[chunk]);
                }
            }
        }
        else if (Get_Block(tile)) {
            Blocks.insert(tile);
            SeesAir[tile.x][tile.y][tile.z] |= 1 << i;
            
            if (lightBlocks) {
                Set_Light(position, SUN_LIGHT_LEVEL);
            }
            
            LightQueue.emplace(Position, position);
        }
    }
    
    Light(false);
    Mesh();
    
    for (auto const &chunk : meshingList) {
        chunk->Light();
        chunk->Mesh();
    }
}

void Chunk::Add_Block(glm::ivec3 position, glm::vec3 diff, int blockType) {
	Set_Block(position, blockType);
	Blocks.insert(position);
    
    if (ChangedBlocks[Position].count(position) && ChangedBlocks[Position][position] == 0) {
        ChangedBlocks[Position].erase(position);
    }
    else {
        ChangedBlocks[Position][position] = blockType;
    }
    
    if (Position.y * CHUNK_SIZE + position.y > topBlocks[Position.xz()][position.xz()]) {
        topBlocks[Position.xz()][position.xz()] = int(Position.y * CHUNK_SIZE + position.y);
        Set_Light(position, SUN_LIGHT_LEVEL);
    }

	std::vector<Chunk*> meshingList;
	std::vector<std::pair<glm::vec3, glm::vec3>> neighbors = Get_Neighbors(Position, position);

	for (int i = 0; i < 6; i++) {
		glm::vec3 chunk = neighbors[i].first;
		glm::uvec3 tile = neighbors[i].second;

		if (Exists(chunk)) {
			if (ChunkMap[chunk]->Get_Block(tile)) {
				ChunkMap[chunk]->SeesAir[tile.x][tile.y][tile.z] &= ~(1 << i);

                if (chunk != Position) {
                    meshingList.push_back(ChunkMap[chunk]);
                }
			}
			else {
				int direction = i + 1;

				if (i % 2 != 0) {
					direction = i - 1;
				}

				SeesAir[position.x][position.y][position.z] |= 1 << direction;
			}
		}
	}
	
    Light();
	Mesh();

    for (auto const &chunk : meshingList) {
        chunk->Light();
        chunk->Mesh();
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
        results.push_back(Get_Chunk_Pos(worldPos + neighborOffsets[i]));
    }

    return results;
}

std::pair<glm::vec3, glm::vec3> Get_Chunk_Pos(glm::vec3 worldPos) {
    glm::vec3 chunk(floor(worldPos.x / CHUNK_SIZE), floor(worldPos.y / CHUNK_SIZE), floor(worldPos.z / CHUNK_SIZE));
    glm::vec3 tile(floor(worldPos.x - (chunk.x * CHUNK_SIZE)), floor(worldPos.y - (chunk.y * CHUNK_SIZE)), floor(worldPos.z - (chunk.z * CHUNK_SIZE)));

    return std::make_pair(chunk, tile);
}

bool Is_Block(glm::vec3 pos) {
    glm::vec3 chunk, tile;
    std::tie(chunk, tile) = Get_Chunk_Pos(pos);
    return Exists(chunk) && ChunkMap[chunk]->Get_Block(tile) > 0;
}