#include "Chunk.h"

#include <chrono>
#include <random>
#include <noise/noise.h>

#include "main.h"
#include "Blocks.h"
#include "Worlds.h"
#include "Interface.h"

class LightNodeComparator {
  public:
    bool operator() (const LightNode &a, const LightNode &b) const {
        for (int i = 0; i < 3; ++i) {
            if (a.Chunk[i] != b.Chunk[i]) {
                return a.Chunk[i] < b.Chunk[i];
            }
        }

        for (int i = 0; i < 3; ++i) {
            if (b.Tile[i] != b.Tile[i]) {
                return a.Tile[i] < b.Tile[i];
            }
        }

        return false;
    }
};

struct MultiBlockMember {
    glm::ivec3 Pos;
    int Type;
    int Data;

    MultiBlockMember(glm::ivec3 pos, int type, int data) {
        Pos = pos;
        Type = type;
        Data = data;
    }
};

class MultiBlockComparator {
  public:
    bool operator () (const MultiBlockMember &a, const MultiBlockMember &b) const {
        if (a.Pos.y != b.Pos.y) {
            return a.Pos.y < b.Pos.y;
        }
        if (a.Pos.x != b.Pos.x) {
            return a.Pos.x < b.Pos.x;
        }
        if (a.Pos.z != b.Pos.z) {
            return a.Pos.z < b.Pos.z;
        }
        return false;
    }
};

const std::map<int, glm::dvec2> OreRanges = {
    {1, {-0.81, -0.80}}, // Coal Ore
    {2, { 1.20,  1.30}}, // Iron Ore
};

const glm::vec3 AOOffsets[6][2][2][3]={
    {{{{-1,-1,0},{-1,0,-1},{-1,-1,-1}},{{-1,1,0},{-1,0,-1},{-1,1,-1}}},
    {{{-1,-1,0},{-1,0,1},{-1,-1,1}},{{-1,1,0},{-1,0,1},{-1,1,1}}}},
    {{{{1,-1,0},{1,0,-1},{1,-1,-1}},{{1,1,0},{1,0,-1},{1,1,-1}}},
    {{{1,-1,0},{1,0,1},{1,-1,1}},{{1,1,0},{1,0,1},{1,1,1}}}},
    {{{{-1,-1,0},{0,-1,-1},{-1,-1,-1}},{{-1,-1,0},{0,-1,1},{-1,-1,1}}},
    {{{1,-1,0},{0,-1,-1},{1,-1,-1}},{{1,-1,0},{0,-1,1},{1,-1,1}}}},
    {{{{-1,1,0},{0,1,-1},{-1,1,-1}},{{-1,1,0},{0,1,1},{-1,1,1}}},
    {{{1,1,0},{0,1,-1},{1,1,-1}},{{1,1,0},{0,1,1},{1,1,1}}}},
    {{{{0,-1,-1},{-1,0,-1},{-1,-1,-1}},{{0,1,-1},{-1,0,-1},{-1,1,-1}}},
    {{{0,-1,-1},{1,0,-1},{1,-1,-1}},{{0,1,-1},{1,0,-1},{1,1,-1}}}},
    {{{{0,-1,1},{-1,0,1},{-1,-1,1}},{{0,1,1},{-1,0,1},{-1,1,1}}},
    {{{0,-1,1},{1,0,1},{1,-1,1}},{{0,1,1},{1,0,1},{1,1,1}}}}
};

static noise::module::RidgedMulti ridgedNoise;
static noise::module::RidgedMulti oreNoise;
static noise::module::Perlin noiseModule;

static std::map<
    glm::vec3, std::set<LightNode, LightNodeComparator>, ChunkPosComparator
> UnloadedLightQueue;

std::map<
    glm::vec2, std::map<glm::vec2, int, VectorComparator>, VectorComparator
> TopBlocks;

std::map<
    glm::vec3, std::map<glm::vec3, std::pair<int, int>, VectorComparator>, ChunkPosComparator
> ChangedBlocks;

static std::mt19937_64 rng;
static std::uniform_real_distribution<double> rngVal(0, 1);

double RNG() {
    return rngVal(rng);
}

void Chunks::Seed(int seed) {
    if (seed == 0) {
        int64_t timeSeed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        std::seed_seq ss {
            static_cast<uint32_t>(timeSeed & 0xffffffff),
            static_cast<uint32_t>(timeSeed >> 32)
        };
        rng.seed(ss);

        std::uniform_int_distribution<int> uni(-2147483648, 2147483647);
        seed = uni(rng);
    }

    WORLD_SEED = seed;

    noiseModule.SetSeed(seed);
    noiseModule.SetPersistence(0.5);
    noiseModule.SetOctaveCount(3);

    ridgedNoise.SetSeed(seed);
    ridgedNoise.SetOctaveCount(2);
    ridgedNoise.SetFrequency(5.0);

    oreNoise.SetSeed(seed);
    oreNoise.SetFrequency(2.0);
}

void Chunk::Update_Air(glm::ivec3 pos, glm::bvec3 inChunk) {
    bool chunkTests[3] = { inChunk.y && inChunk.z, inChunk.x && inChunk.z, inChunk.x && inChunk.y };
    static glm::ivec3 offsets[3] = { glm::ivec3(1, 0, 0), glm::ivec3(0, 1, 0), glm::ivec3(0, 0, 1) };

    for (int i = 0; i < 3; ++i) {
        if (!chunkTests[i]) {
            continue;
        }

        if (pos[i] < CHUNK_SIZE - 1) {
            Get_Air_Ref(pos + offsets[i]) |= 1 << (i * 2);
        }
        if (pos[i] > 0) {
            Get_Air_Ref(pos - offsets[i]) |= 1 << (i * 2 + 1);
        }
    }
}

void Chunk::Update_Transparency(glm::ivec3 pos) {
    int index = 0;
    for (auto const &neighbor : Get_Neighbors(Position, pos)) {
        if (!Exists(neighbor.first) ||
            !ChunkMap[neighbor.first]->TransparentBlocks.count(neighbor.second))
        {
            ++index;
            continue;
        }

        ChunkMap[neighbor.first]->Get_Air_Ref(neighbor.second) &= ~(1 << index);
        Get_Air_Ref(pos) &= ~(1 << (index + ((index % 2) ? -1 : 1)));
        ++index;
    }
}

void Chunk::Check_Ore(glm::ivec3 pos, glm::dvec3 noisePos) {
    double value = oreNoise.GetValue(noisePos.x, noisePos.y, noisePos.z);

    for (auto const &range : OreRanges) {
        if (value >= range.second.x && value <= range.second.y) {
            Set_Type(pos, 15);
            Set_Data(pos, range.first);
            return;
        }
    }

    Set_Type(pos, 1);
}

void Chunk::Generate() {
    glm::vec2 topPos = Position.xz();
    glm::dvec3 positionOffset = static_cast<glm::dvec3>(Position);
    positionOffset *= static_cast<double>(CHUNK_SIZE);

    bool underground = Position.y < -3;
    double densityThreshold = underground ?
                              NOISE_DENSITY_CAVE :
                              NOISE_DENSITY_BLOCK;

    if (Position.y == 3) {
        TopBlocks[topPos].clear();
    }

    for (int x = -1; x <= CHUNK_SIZE; ++x) {
        for (int z = -1; z <= CHUNK_SIZE; ++z) {
            for (int y = CHUNK_SIZE; y >= -1; --y) {
                glm::ivec3 block(x, y, z);
                glm::bvec3 inChunk = glm::equal(
                    glm::greaterThan(block, glm::ivec3(-1)),
                    glm::lessThan(block, glm::ivec3(CHUNK_SIZE))
                );
                int height = static_cast<int>(Position.y) * CHUNK_SIZE + y;

                if (Blocks.count(block)) {
                    continue;
                }

                if (ChangedBlocks.count(Position) && ChangedBlocks[Position].count(block)) {
                    int type, data;
                    std::tie(type, data) = ChangedBlocks[Position][block];

                    if (type == 0) {
                        Update_Air(block, inChunk);
                        continue;
                    }

                    const Block* blockInstance = Blocks::Get_Block(type, data);

                    if (!Top_Exists(block) || height > Get_Top(block)) {
                        Set_Top(block, height);
                        Set_Light(block, SUN_LIGHT_LEVEL);
                        LightQueue.emplace(Position, block);
                    }

                    Set_Type(block, type);
                    Set_Data(block, data);

                    if (!blockInstance->FullBlock || blockInstance->Transparent) {
                        ContainsTransparentBlocks = true;
                        Update_Air(block, inChunk);
                    }

                    if (blockInstance->Transparent) {
                        TransparentBlocks.insert(block);
                        Update_Transparency(block);
                    }

                    Blocks.insert(block);
                    continue;
                }

                glm::dvec3 nPos = (positionOffset + glm::dvec3(x, y, z));
                nPos /= static_cast<double>(CHUNK_ZOOM);

                double noiseValue = underground ?
                    ridgedNoise.GetValue(nPos.x, nPos.y, nPos.z) :
                    noiseModule.GetValue(nPos.x, nPos.y, nPos.z) - nPos.y * 2;

                if (noiseValue < densityThreshold) {
                    Update_Air(block, inChunk);
                    continue;
                }

                if (inChunk != glm::bvec3(true)) {
                    continue;
                }

                if (!Top_Exists(block) || height > Get_Top(block)) {
                    Set_Top(block, height);
                    Set_Light(block, SUN_LIGHT_LEVEL);
                    LightQueue.emplace(Position, block);

                    Set_Type(block, 2);
                    // Generate_Tree(block);
                }
                else {
                    int depth = std::abs(
                        TopBlocks[topPos][block.xz()] - height
                    );

                    if (depth > 3) {
                        underground ? Check_Ore(block, nPos) : Set_Type(block, 1);
                    }
                    else {
                        Set_Type(block, 3);
                    }
                }

                Blocks.insert(block);
            }
        }
    }

    Generated = true;
}

void Chunk::Generate_Tree(glm::vec3 tile) {
    static std::map<glm::ivec3, std::pair<int, int>, VectorComparator> TreeModel = {
        {{0, 1, 0}, {17, 0}},
        {{0, 2, 0}, {17, 0}},
        {{0, 3, 0}, {17, 0}},
        {{0, 4, 0}, {17, 0}},
        {{0, 5, 0}, {17, 0}},

        {{-2, 4, -2}, {18, 0}},
        {{-1, 4, -2}, {18, 0}},
        {{ 0, 4, -2}, {18, 0}},
        {{ 1, 4, -2}, {18, 0}},
        {{ 2, 4, -2}, {18, 0}},

        {{-2, 4, -1}, {18, 0}},
        {{-1, 4, -1}, {18, 0}},
        {{ 0, 4, -1}, {18, 0}},
        {{ 1, 4, -1}, {18, 0}},
        {{ 2, 4, -1}, {18, 0}},

        {{-2, 4,  0}, {18, 0}},
        {{-1, 4,  0}, {18, 0}},
        {{ 1, 4,  0}, {18, 0}},
        {{ 2, 4,  0}, {18, 0}},

        {{-2, 4,  1}, {18, 0}},
        {{-1, 4,  1}, {18, 0}},
        {{ 0, 4,  1}, {18, 0}},
        {{ 1, 4,  1}, {18, 0}},
        {{ 2, 4,  1}, {18, 0}},

        {{-2, 4,  2}, {18, 0}},
        {{-1, 4,  2}, {18, 0}},
        {{ 0, 4,  2}, {18, 0}},
        {{ 1, 4,  2}, {18, 0}},
        {{ 2, 4,  2}, {18, 0}},


        {{-2, 5, -2}, {18, 0}},
        {{-1, 5, -2}, {18, 0}},
        {{ 0, 5, -2}, {18, 0}},
        {{ 1, 5, -2}, {18, 0}},
        {{ 2, 5, -2}, {18, 0}},

        {{-2, 5, -1}, {18, 0}},
        {{-1, 5, -1}, {18, 0}},
        {{ 0, 5, -1}, {18, 0}},
        {{ 1, 5, -1}, {18, 0}},
        {{ 2, 5, -1}, {18, 0}},

        {{-2, 5,  0}, {18, 0}},
        {{-1, 5,  0}, {18, 0}},
        {{ 1, 5,  0}, {18, 0}},
        {{ 2, 5,  0}, {18, 0}},

        {{-2, 5,  1}, {18, 0}},
        {{-1, 5,  1}, {18, 0}},
        {{ 0, 5,  1}, {18, 0}},
        {{ 1, 5,  1}, {18, 0}},
        {{ 2, 5,  1}, {18, 0}},

        {{-2, 5,  2}, {18, 0}},
        {{-1, 5,  2}, {18, 0}},
        {{ 0, 5,  2}, {18, 0}},
        {{ 1, 5,  2}, {18, 0}},
        {{ 2, 5,  2}, {18, 0}},

        {{-1, 6,  0}, {18, 0}},
        {{ 1, 6,  0}, {18, 0}},
        {{ 0, 6,  0}, {18, 0}},
        {{ 0, 6, -1}, {18, 0}},
        {{ 0, 6,  1}, {18, 0}},

        {{-1, 7,  0}, {18, 0}},
        {{ 1, 7,  0}, {18, 0}},
        {{ 0, 7,  0}, {18, 0}},
        {{ 0, 7, -1}, {18, 0}},
        {{ 0, 7,  1}, {18, 0}}
    };

    if (RNG() <= 0.2) {
        glm::ivec3 root = Get_World_Pos(Position, tile);

        for (int y = 1; y <= 4; ++y) {
            glm::ivec3 pos = root + glm::ivec3(0, y, 0);
            glm::ivec3 chunkPos, tilePos;
            std::tie(chunkPos, tilePos) = Get_Chunk_Pos(pos);

            if (!Exists(chunkPos) || ChunkMap[chunkPos]->Get_Type(tilePos) != 0) {
                return;
            }
        }

        for (auto const &block : TreeModel) {
            glm::ivec3 pos = root + block.first;
            glm::ivec3 chunkPos, tilePos;
            std::tie(chunkPos, tilePos) = Get_Chunk_Pos(pos);

            if (!Exists(chunkPos)) {
                continue;
            }

            if (ChunkMap[chunkPos]->Get_Type(tilePos) == 0) {
                ChunkMap[chunkPos]->Set_Type(tilePos, block.second.first);
                ChunkMap[chunkPos]->Set_Data(tilePos, block.second.second);
                ChunkMap[chunkPos]->Blocks.insert(tilePos);
            }
        }
    }
}

bool Check_If_Node(LightNode node) {
    Chunk* c = ChunkMap[node.Chunk];

    if (c->Get_Light(node.Tile) + 1 >= node.LightLevel || !c->Get_Air(node.Tile)) {
        return false;
    }

    c->Set_Light(node.Tile, node.LightLevel - (!node.Down));
    return true;
}

void Chunk::Light(bool flag) {
    if (UnloadedLightQueue.count(Position)) {
        for (auto node : UnloadedLightQueue[Position]) {
            if (Check_If_Node(node)) {
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
        int lightLevel = node.LightLevel;

        if (!ChunkMap[chunk]->Get_Air(tile)) {
            continue;
        }

        for (auto const &neighbor : Get_Neighbors(chunk, tile)) {
            if (!Exists(neighbor.first)) {
                continue;
            }

            Chunk* neighborChunk = ChunkMap[neighbor.first];

            if (!neighborChunk->Get_Air(neighbor.second)) {
                continue;
            }

            int neighborLight = neighborChunk->Get_Light(neighbor.second);

            if (neighborLight == 0 && lightLevel > 0) {
                continue;
            }

            if (neighborLight > 0 && neighborLight < lightLevel) {
                neighborChunk->Set_Light(neighbor.second, 0);
                neighborChunk->LightRemovalQueue.emplace(neighbor.first, neighbor.second, neighborLight);
            }
            else {
                neighborChunk->Set_Light(neighbor.second, neighborLight);
                neighborChunk->LightQueue.emplace(neighbor.first, neighbor.second);
            }

            neighborChunk->Meshed = false;
        }
    }

    while (!LightQueue.empty()) {
        LightNode node = LightQueue.front();
        LightQueue.pop();

        glm::vec3 tile = node.Tile;
        int lightLevel = Get_Light(tile);
        bool visible = Get_Air(tile) > 0;

        int index = 0;

        for (auto const &neighbor : Get_Neighbors(Position, tile)) {
            LightNode newNode(
                neighbor.first, neighbor.second,
                lightLevel, index == UP
            );

            if (!ChunkMap.count(neighbor.first)) {
                UnloadedLightQueue[neighbor.first].insert(newNode);
            }

            else if (visible && Check_If_Node(newNode)) {
                ChunkMap[neighbor.first]->LightQueue.emplace(
                    neighbor.first, neighbor.second
                );

                if (neighbor.first != Position && flag) {
                    ChunkMap[neighbor.first]->Meshed = false;
                }
            }
        }

        ++index;
    }
}

float Chunk::GetAO(glm::vec3 block, int face, int index) {
    float ao = 0.0f;

    glm::ivec2 vertexIndexes[3] = {
        vertices[face][index].zy(),
        vertices[face][index].xz(),
        vertices[face][index].xy()
    };

    for (int i = 0; i < 3; ++i) {
        glm::ivec2 vertexIndex = vertexIndexes[face / 2];

        if (!Blocks.count(
            block + AOOffsets[face][vertexIndex.x][vertexIndex.y][i]
        )) {
            continue;
        }

        if (ao == 1 && i == 1) {
            return 0.0f;
        }

        ++ao;
    }

    return ao;
}

int Chunk::Get_Extra_Texture(glm::ivec3 tile) {
    if (ExtraTextures.count(tile)) {
        return ExtraTextures[tile];
    }

    return 0;
}

void Chunk::Mesh() {
    VBOData.clear();

    std::set<glm::vec3>::iterator block = Blocks.begin();

    while (block != Blocks.end()) {
        unsigned char seesAir = Get_Air(*block);

        if (seesAir == 0) {
            block = Blocks.erase(block);
            continue;
        }

        glm::vec3 posOffset = *block + Position * static_cast<float>(CHUNK_SIZE);
        float lightValue = static_cast<float>(Get_Light(*block));
        const Block* blockInstance = Blocks::Get_Block(Get_Type(*block), Get_Data(*block));

        if (blockInstance->HasCustomData) {
            for (auto const &element : blockInstance->CustomData) {
                for (unsigned long i = 0; i < 6; i++) {
                    for (unsigned long j = 0; j < 6; j++) {
                        Extend(VBOData, element[i][j].first + posOffset);
                        Extend(VBOData, element[i][j].second);
                        VBOData.push_back(lightValue);
                        VBOData.push_back(0);
                        VBOData.push_back(static_cast<float>(Get_Extra_Texture(*block)));
                    }
                }
            }
        }
        else {
            for (int bit = 0; bit < 6; ++bit, seesAir >>= 1) {
                if (!(seesAir & 1)) {
                    continue;
                }

                for (int j = 0; j < 6; j++) {
                    Extend(VBOData, vertices[bit][j] + posOffset);

                    if (blockInstance->MultiTextures) {
                        Extend(VBOData, tex_coords[bit][j]);
                        VBOData.push_back(static_cast<float>(blockInstance->Textures[static_cast<unsigned long>(bit)]));
                    }
                    else {
                        Extend(VBOData, tex_coords[bit][j]);
                        VBOData.push_back(static_cast<float>(blockInstance->Texture));
                    }

                    VBOData.push_back(lightValue);

                    if (AMBIENT_OCCLUSION) {
                        VBOData.push_back(GetAO(*block, bit, j));
                    }
                    else {
                        VBOData.push_back(0);
                    }

                    VBOData.push_back(static_cast<float>(Get_Extra_Texture(*block)));
                }
            }
        }

        ++block;
    }

    if (VBOData.size() > 0) {
        Meshed = true;
    }

    DataUploaded = false;
}

void Chunk::Draw(bool transparentPass) {
    if (!Meshed) {
        return;
    }

    if (!DataUploaded) {
        buffer.Upload(VBOData);
        DataUploaded = true;
    }

    if (Visible) {
        if (!transparentPass || ContainsTransparentBlocks) {
            buffer.Draw();
        }
    }
}

void Chunk::Remove_Multiblock(glm::ivec3 position, const Block* block) {
    glm::ivec3 root;
    glm::ivec3 worldPos = Get_World_Pos(Position, position);
    std::set<glm::ivec3, VectorComparator> IterList = {worldPos};
    std::set<glm::ivec3, VectorComparator> MultiBlockParts = {worldPos};

    if (block->IsMBRoot) {
        root = worldPos;
    }
    else {
        root = worldPos - block->MBOffset;
    }

    std::set<glm::ivec3>::iterator part = IterList.begin();

    while (part != IterList.end()) {
        glm::vec3 chunk, tile;
        std::tie(chunk, tile) = Get_Chunk_Pos(*part);

        for (auto const &neighbor : Get_Neighbors(chunk, tile)) {
            glm::ivec3 pos = Get_World_Pos(neighbor.first, neighbor.second);

            if (pos == root) {
                IterList.insert(pos);
                MultiBlockParts.insert(pos);
            }
            else {
                const Block* partBlock = Blocks::Get_Block(
                    ChunkMap[neighbor.first]->Get_Type(neighbor.second),
                    ChunkMap[neighbor.first]->Get_Data(neighbor.second)
                );

                if (partBlock->MultiBlock) {
                    if (root + partBlock->MBOffset == pos) {
                        IterList.insert(pos);
                        MultiBlockParts.insert(pos);
                    }
                }
            }
        }

        part = IterList.erase(part);
    }

    for (auto const &partBlock : MultiBlockParts) {
        glm::ivec3 chunk, tile;
        std::tie(chunk, tile) = Get_Chunk_Pos(partBlock);
        ChunkMap[chunk]->Remove_Block(tile, false);
    }
}

void Chunk::Add_Multiblock(glm::ivec3 position, const Block* block) {
    glm::ivec3 root = Get_World_Pos(Position, position);
    std::set<MultiBlockMember, MultiBlockComparator> BlocksToAdd;

    for (auto const &typePair : block->Types) {
        const Block &type = typePair.second;
        glm::ivec3 blockPos = root + type.MBOffset;
        glm::ivec3 chunk, tile;
        std::tie(chunk, tile) = Get_Chunk_Pos(blockPos);

        if (Exists(chunk) && ChunkMap[chunk]->Get_Type(tile) == 0) {
            BlocksToAdd.emplace(MultiBlockMember(blockPos, type.ID, type.Data));
        }
        else {
            return;
        }
    }

    for (auto const &b : BlocksToAdd) {
        glm::ivec3 chunk, tile;
        std::tie(chunk, tile) = Get_Chunk_Pos(b.Pos);
        ChunkMap[chunk]->Add_Block(tile, b.Type, b.Data, false);
    }
}

void Chunk::Remove_Block(glm::ivec3 position, bool checkMulti) {
    const Block* block = Blocks::Get_Block(Get_Type(position), Get_Data(position));

    if (checkMulti && block->MultiBlock) {
        Remove_Multiblock(position, block);
        return;
    }

    Set_Type(position, 0);
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

    Worlds::Save_Chunk(WORLD_NAME, Position);

    bool lightBlocks = false;

    if (TopBlocks[Position.xz()][position.xz()] == Position.y * CHUNK_SIZE + position.y) {
        lightBlocks = true;
        TopBlocks[Position.xz()][position.xz()]--;
    }

    std::vector<Chunk*> meshingList;
    std::vector<std::pair<glm::vec3, glm::vec3>> neighbors = Get_Neighbors(Position, position);

    for (unsigned long i = 0; i < 6; i++) {
        glm::vec3 chunk = neighbors[i].first;
        glm::uvec3 tile = neighbors[i].second;

        if (chunk != Position) {
            if (Exists(chunk)) {
                if (ChunkMap[chunk]->Get_Type(tile)) {
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
        else if (Get_Type(tile)) {
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

void Chunk::Add_Block(glm::ivec3 position, int blockType, int blockData, bool checkMulti) {
    const Block* block = Blocks::Get_Block(blockType, blockData);

    if (checkMulti && block->MultiBlock) {
        Add_Multiblock(position, block);
        return;
    }

    Set_Type(position, blockType);
    Set_Data(position, blockData);
    Blocks.insert(position);

    if (ChangedBlocks[Position].count(position) && ChangedBlocks[Position][position] == std::pair<int, int>(0, 0)) {
        ChangedBlocks[Position].erase(position);
    }
    else {
        ChangedBlocks[Position][position] = std::make_pair(blockType, blockData);
    }

    Worlds::Save_Chunk(WORLD_NAME, Position);

    if (Position.y * CHUNK_SIZE + position.y > TopBlocks[Position.xz()][position.xz()]) {
        TopBlocks[Position.xz()][position.xz()] = static_cast<int>(Position.y * CHUNK_SIZE + position.y);
        Set_Light(position, SUN_LIGHT_LEVEL);
    }

    std::vector<Chunk*> meshingList;

    if (block->FullBlock && !block->Transparent) {
        std::vector<std::pair<glm::vec3, glm::vec3>> neighbors = Get_Neighbors(Position, position);
        glm::vec3 chunk, tile;

        for (int i = 0; i < 6; i++) {
            std::tie(chunk, tile) = neighbors[static_cast<unsigned long>(i)];

            if (Exists(chunk)) {
                if (ChunkMap[chunk]->Get_Type(tile)) {
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
            {1, 0, 0}, {-1, 0, 0},
            {0, 1, 0}, {0, -1, 0},
            {0, 0, 1}, {0, 0, -1}
    };

    for (int i = 0; i < 6; i++) {
        results.push_back(Get_Chunk_Pos(worldPos + neighborOffsets[i]));
    }

    return results;
}

std::pair<glm::vec3, glm::vec3> Get_Chunk_Pos(glm::vec3 worldPos) {
    glm::vec3 chunk = glm::floor(worldPos / static_cast<float>(CHUNK_SIZE));
    glm::vec3 tile = glm::floor(worldPos - (chunk * static_cast<float>(CHUNK_SIZE)));
    return std::make_pair(chunk, tile);
}

bool Is_Block(glm::vec3 pos) {
    glm::vec3 chunk, tile;
    std::tie(chunk, tile) = Get_Chunk_Pos(pos);
    return Exists(chunk) && ChunkMap[chunk]->Get_Type(tile) > 0;
}

bool Exists(glm::vec3 chunk) {
    return ChunkMap.count(chunk) && ChunkMap[chunk]->DataUploaded;
}
