#include "Chunk.h"

#include <random>
#include <noise/noise.h>

#include "Blocks.h"
#include "Interface.h"

const int CHUNK_ZOOM = 50;
const float NOISE_DENSITY_BLOCK = 0.5f;
const float NOISE_DENSITY_CAVE = -0.85f;

const std::map<int, glm::vec2> OreRanges = {
    {1, glm::vec2(-0.81, -0.80)}, // Coal Ore
    {2, glm::vec2( 1.20,  1.30)}, // Iron Ore
};

bool Seeded = false;

noise::module::Perlin noiseModule;
noise::module::RidgedMulti ridgedNoise;
noise::module::RidgedMulti oreNoise;

enum Directions {LEFT, RIGHT, DOWN, UP, BACK, FRONT};

const glm::vec3 offsets[6][2][2][3] = {
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

std::map<glm::vec2, std::map<glm::vec2, int, Vec2Comparator>, Vec2Comparator> topBlocks;
std::map<glm::vec3, std::set<LightNode, LightNodeComparator>, Vec3Comparator> UnloadedLightQueue;
std::map<glm::vec3, std::map<glm::vec3, std::pair<int, int>, Vec3Comparator>, Vec3Comparator> ChangedBlocks;

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
    
    oreNoise.SetSeed(uni(rng));
    oreNoise.SetFrequency(2.0);
}

Chunk::Chunk(glm::vec3 position) {
    if (!Seeded) {
        Seed();
    }
    
    Position = position;
}

void Chunk::Update_Air(glm::ivec3 pos, glm::bvec3 inChunk) {
	bool chunkTests[3] = { inChunk.y && inChunk.z, inChunk.x && inChunk.z, inChunk.x && inChunk.y };
	static glm::ivec3 offsets[3] = { glm::ivec3(1, 0, 0), glm::ivec3(0, 1, 0), glm::ivec3(0, 0, 1) };

	for (int i = 0; i < 3; i++) {
        if (chunkTests[i]) {
            if (pos[i] < CHUNK_SIZE - 1) {
                Get_Air_Ref(pos + offsets[i]) |= 1 << (i * 2);
            }
            if (pos[i] > 0) {
                Get_Air_Ref(pos - offsets[i]) |= 1 << (i * 2 + 1);
            }
        }
	}
}

void Chunk::Update_Transparency(glm::ivec3 pos) {
    std::vector<std::pair<glm::vec3, glm::vec3>> neighbors = Get_Neighbors(Position, pos);
    
    int index = 0;
    for (auto const &neighbor : neighbors) {
        if (Exists(neighbor.first)) {
            if (ChunkMap[neighbor.first]->TransparentBlocks.count(neighbor.second)) {
                ChunkMap[neighbor.first]->Get_Air_Ref(neighbor.second) &= ~(1 << index);
                Get_Air_Ref(pos) &= ~(1 << (index + ((index % 2) ? -1 : 1)));
            }
        }
        
        ++index;
    }
}

void Chunk::Check_Ore(glm::ivec3 pos, glm::vec3 noisePos) {
    double value = oreNoise.GetValue(noisePos.x, noisePos.y, noisePos.z);
    
    for (auto const &range : OreRanges) {
        if (value >= range.second.x && value <= range.second.y) {
            Set_Block(pos, 15);
            Set_Data(pos, range.first);
            return;
        }
    }
    
    Set_Block(pos, 1);
}

void Chunk::Generate() {
    glm::vec2 topPos = Position.xz();
    glm::vec3 positionOffset = Position * float(CHUNK_SIZE);
    
    bool underground = Position.y < -3;
    float densityThreshold = underground ? NOISE_DENSITY_CAVE : NOISE_DENSITY_BLOCK;
    
    if (Position.y == 3) {
        topBlocks[topPos].clear();
    }
    
    for (int x = -1; x <= CHUNK_SIZE; x++) {
        for (int z = -1; z <= CHUNK_SIZE; z++) {
            for (int y = CHUNK_SIZE; y >= -1; y--) {
                glm::ivec3 block(x, y, z);

				glm::bvec3 inChunk = glm::bvec3(
					x >= 0 && x < CHUNK_SIZE,
					y >= 0 && y < CHUNK_SIZE,
					z >= 0 && z < CHUNK_SIZE
				);
                
                if (ChangedBlocks.count(Position) && ChangedBlocks[Position].count(block)) {
                    int type, data;
                    std::tie(type, data) = ChangedBlocks[Position][block];
                    
                    if (type > 0) {
                        const Block* blockInstance = Blocks::Get_Block(type, data);
                        
                        if (!topBlocks[topPos].count(block.xz())) {
                            topBlocks[topPos][block.xz()] = Position.y * CHUNK_SIZE + y;
                            TopBlocks.insert(block);
                            
                            Set_Block(block, type);
                            Set_Data(block, data);
                            Blocks.insert(block);
                        }
                        
                        Set_Block(block, type);
                        Set_Data(block, data);
                        Blocks.insert(block);
                        
                        if (!blockInstance->FullBlock || blockInstance->Transparent) {
                            ContainsTransparentBlocks = true;
                            Update_Air(block, inChunk);
                        }
                        
                        if (blockInstance->Transparent) {
                            TransparentBlocks.insert(block);
                            Update_Transparency(block);
                        }
                    }
                    else {
                        Update_Air(block, inChunk);
                    }
                }
                else {
                    glm::vec3 nPos = (positionOffset + glm::vec3(x, y, z)) / float(CHUNK_ZOOM);
                    double noiseValue = underground ? ridgedNoise.GetValue(nPos.x, nPos.y, nPos.z) : (noiseModule.GetValue(nPos.x, nPos.y, nPos.z) - nPos.y * 2);
                    
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
                                    underground ? Check_Ore(block, nPos) : Set_Block(block, 1);
                                }
                                else {
                                    Set_Block(block, 3);
                                }
                            }
                            
                            Blocks.insert(block);
                        }
                    }
                    else {
                        Update_Air(block, inChunk);
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

	int vertexIndex[3][2] = { {int(vertices[face][index].z), int(vertices[face][index].y)}, {int(vertices[face][index].x), int(vertices[face][index].z)}, {int(vertices[face][index].x), int(vertices[face][index].y)} };

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
        unsigned char seesAir = Get_Air(*block);

        if (seesAir == 0) {
            block = Blocks.erase(block);
        }
        else {
            int bit = 0;
            float lightValue = float(Get_Light(*block));
            const Block* blockInstance = Blocks::Get_Block(Get_Block(*block), Get_Data(*block));
            
            while (bit < 6) {
                if (seesAir & 1) {
                    for (int j = 0; j < 6; j++) {
                        if (blockInstance->CustomVertices) {
                            VBOData.push_back(blockInstance->Vertices[bit][vertices[bit][j].x].x + block->x + Position.x * CHUNK_SIZE);
                            VBOData.push_back(blockInstance->Vertices[bit][vertices[bit][j].y].y + block->y + Position.y * CHUNK_SIZE);
                            VBOData.push_back(blockInstance->Vertices[bit][vertices[bit][j].z].z + block->z + Position.z * CHUNK_SIZE);
                        }
                        else {
                            Extend(VBOData, vertices[bit][j] + (*block) + Position * float(CHUNK_SIZE));
                        }
                        
                        if (blockInstance->CustomTexCoords) {
                            VBOData.push_back(blockInstance->TexCoords[bit][tex_coords[bit][j].x].x / IMAGE_SIZE.x);
                            VBOData.push_back(blockInstance->TexCoords[bit][tex_coords[bit][j].y].y / IMAGE_SIZE.y);
                        }
						else if (blockInstance->MultiTextures) {
                            Extend(VBOData, (blockInstance->Textures[bit] - 1.0f + tex_coords[bit][j]) / IMAGE_SIZE);
						}
						else {
                            Extend(VBOData, (blockInstance->Texture - 1.0f + tex_coords[bit][j]) / IMAGE_SIZE);
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
    Set_Data(position, 0);
    Blocks.erase(position);
    
    if (TransparentBlocks.count(position)) {
        TransparentBlocks.erase(position);
        
        // Checks if chunk still contains transparent blocks
        if (ContainsTransparentBlocks && TransparentBlocks.empty()) {
            ContainsTransparentBlocks = false;
        }
    }
    
    if (ChangedBlocks[Position].count(position)) {
        ChangedBlocks[Position].erase(position);
    }
    else {
        ChangedBlocks[Position][position] = std::make_pair(0, 0);
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
                    ChunkMap[chunk]->Get_Air_Ref(tile) |= 1 << i;
                    
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
            Get_Air_Ref(tile) |= 1 << i;
            
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

void Chunk::Add_Block(glm::ivec3 position, glm::vec3 diff, int blockType, int blockData) {
	Set_Block(position, blockType);
    Set_Data(position, blockData);
	Blocks.insert(position);
    
    if (ChangedBlocks[Position].count(position) && ChangedBlocks[Position][position] == std::pair<int, int>(0, 0)) {
        ChangedBlocks[Position].erase(position);
    }
    else {
        ChangedBlocks[Position][position] = std::make_pair(blockType, blockData);
    }
    
    if (Position.y * CHUNK_SIZE + position.y > topBlocks[Position.xz()][position.xz()]) {
        topBlocks[Position.xz()][position.xz()] = int(Position.y * CHUNK_SIZE + position.y);
        Set_Light(position, SUN_LIGHT_LEVEL);
    }

	std::vector<Chunk*> meshingList;
    
    const Block* block = Blocks::Get_Block(blockType, blockData);
    
    if (block->FullBlock && !block->Transparent) {
        std::vector<std::pair<glm::vec3, glm::vec3>> neighbors = Get_Neighbors(Position, position);
        glm::vec3 chunk, tile;
        
        for (int i = 0; i < 6; i++) {
            std::tie(chunk, tile) = neighbors[i];
            
            if (Exists(chunk)) {
                if (ChunkMap[chunk]->Get_Block(tile)) {
                    ChunkMap[chunk]->Get_Air_Ref(tile) &= ~(1 << i);
                    
                    if (chunk != Position) {
                        meshingList.push_back(ChunkMap[chunk]);
                    }
                }
                else {
                    Get_Air_Ref(position) |= 1 << (i + ((i % 2) ? -1 : 1));
                }
            }
        }
    }
    else {
        ContainsTransparentBlocks = true;
        
        if (block->Transparent) {
            TransparentBlocks.insert(position);
            Update_Transparency(position);
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

    static glm::vec3 neighborOffsets[6] = {
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