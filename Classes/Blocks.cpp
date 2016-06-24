#include "Blocks.h"

#include <dirent.h>
#include <sstream>
#include <fstream>
#include "json.hpp"

#include "main.h"
#include "Interface.h"

#define BLOCK_LAMBDA(_v) [](Block &b, JSONValue val) { b._v = val; }
#define VECTOR_LAMBDA(_v) [](Block &b, JSONValue val) { \
    for (unsigned long j = 0; j < 3; j++) \
        b._v[static_cast<int>(j)] = val[j]; \
}

typedef nlohmann::basic_json<
    std::__1::map, std::__1::vector, std::__1::basic_string<
        char, std::__1::char_traits<char>, std::__1::allocator<char>
    >, bool, long long, double, std::__1::allocator
> JSONValue;

static std::map<std::string, std::function<void(Block &b, JSONValue val)>> lambdas = {
    {"id",           BLOCK_LAMBDA(ID)           },
    {"name",         BLOCK_LAMBDA(Name)         },
    {"hardness",     BLOCK_LAMBDA(Hardness)     },
    {"transparent",  BLOCK_LAMBDA(Transparent)  },
    {"fullBlock",    BLOCK_LAMBDA(FullBlock)    },
    {"collision",    BLOCK_LAMBDA(Collision)    },
    {"targetable",   BLOCK_LAMBDA(Targetable)   },
    {"luminosity",   BLOCK_LAMBDA(Luminosity)   },
    {"sound",        BLOCK_LAMBDA(Sound)        },
    {"multiBlock",   BLOCK_LAMBDA(MultiBlock)   },
    {"isMBRoot",     BLOCK_LAMBDA(IsMBRoot)     },
    {"mBOffset",     VECTOR_LAMBDA(MBOffset)    },
    {"hitboxScale",  VECTOR_LAMBDA(Scale)       },
    {"hitboxOffset", VECTOR_LAMBDA(ScaleOffset) }
};

static std::map<int, Block> BlockTypes;
static std::map<int, Item>  ItemTypes;

bool Case_Insensitive_Cmp(const std::string &a, const std::string &b) {
    unsigned long sz = a.size();

    if (b.size() != sz) {
        return false;
    }

    for (unsigned long i = 0; i < sz; ++i) {
        if (tolower(a[i]) != tolower(b[i])) {
            return false;
        }
    }

    return true;
}

nlohmann::json Parse_JSON(std::string path) {
    std::stringstream file_content;
    nlohmann::json json;

    std::ifstream file(path);
    file_content << file.rdbuf();
    file.close();

    json << file_content;
    return json;
}

void Process_Block(nlohmann::json::iterator it, Block &block) {
    if (lambdas.count(it.key())) {
        lambdas[it.key()](block, it.value());
    }

    else if (it.key() == "icon") {
        block.HasIcon = true;
        block.Icon = it.value();
    }

    else if (it.key() == "texture") {
        block.HasTexture = true;
        block.Texture = it.value();
    }

    else if (it.key() == "multiTexture") {
        block.MultiTextures = true;

        for (auto const &side : it.value()) {
            block.Textures.push_back(side);
        }
    }

    else if (it.key() == "elements") {
        block.HasCustomData = true;

        glm::vec3 smallestCoord = glm::vec3(10);
        glm::vec3 largestCoord = glm::vec3(0);

        for (auto const &element : it.value()) {
            glm::vec3 startPos(element["from"][0], element["from"][1], element["from"][2]);
            glm::vec3 endPos(element["to"][0], element["to"][1], element["to"][2]);

            for (int i = 0; i < 3; i++) {
                if (startPos[i] < smallestCoord[i]) {
                    smallestCoord[i] = startPos[i];
                }
                if (endPos[i] > largestCoord[i]) {
                    largestCoord[i] = endPos[i];
                }
            }

            int index = 0;

            std::vector<std::vector<std::pair<glm::vec3, glm::vec3>>> elementData;

            for (auto const &texture : element["texCoords"]) {
                std::vector<std::pair<glm::vec3, glm::vec3>> data;

                int textureID;
                glm::vec2 texStart(0);
                glm::vec2 texEnd(1);

                if (texture.is_array()) {
                    textureID = texture[0];
                    texStart = glm::vec2(texture[1], texture[2]);
                    texEnd = glm::vec2(texture[3], texture[4]);
                }
                else {
                    textureID = texture;
                }

                for (int i = 0; i < 6; i++) {
                    data.push_back(std::make_pair(
                        startPos + (endPos - startPos) * vertices[index][i],
                        glm::vec3(
                            texStart + (texEnd - texStart) * tex_coords[index][i],
                            textureID
                        )
                    ));
                }

                elementData.push_back(data);
                ++index;
            }

            block.CustomData.push_back(elementData);
        }

        if (block.Scale == glm::vec3(1)) {
            block.Scale = largestCoord - smallestCoord;
        }

        if (block.ScaleOffset == glm::vec3(0)) {
            block.ScaleOffset = smallestCoord;
        }
    }
}

void Process_Item(nlohmann::json::iterator it, Item &item) {
    if      (it.key() == "id")   { item.ID   = it.value(); }
    else if (it.key() == "name") { item.Name = it.value(); }
    else if (it.key() == "icon") { item.Icon = it.value(); }
}

void Blocks::Init() {
    DIR* blockDir = opendir("BlockData");
    struct dirent* blockEnt;

    std::vector<std::string> sides = {"left", "right", "down", "up", "back", "front"};

    while ((blockEnt = readdir(blockDir)) != nullptr) {
        std::string fileName(blockEnt->d_name);

        if (fileName.find(".json") == std::string::npos) {
            continue;
        }

        Block block;
        nlohmann::json json = Parse_JSON("BlockData/" + fileName);

        for (auto it = json.begin(); it != json.end(); ++it) {
            Process_Block(it, block);
        }

        if (!json.count("types")) {
            BlockTypes[block.ID] = block;
            continue;
        }

        int subIndex = 1;

        for (auto &type : json["types"]) {
            Block subType = block;
            subType.Data = subIndex;

            for (auto it = type.begin(); it != type.end(); ++it) {
                Process_Block(it, subType);
            }

            block.Types[subIndex++] = subType;
        }

        BlockTypes[block.ID] = block;
    }

    closedir(blockDir);

    DIR* itemDir = opendir("ItemData");
    struct dirent* itemEnt;

    while ((itemEnt = readdir(itemDir)) != nullptr) {
        std::string fileName(itemEnt->d_name);

        if (fileName.find(".json") == std::string::npos) {
            continue;
        }

        Item item;
        nlohmann::json json = Parse_JSON("ItemData/" + fileName);

        for (auto it = json.begin(); it != json.end(); ++it) {
            Process_Item(it, item);
        }

        if (!json.count("types")) {
            ItemTypes[item.ID] = item;
            continue;
        }

        int subIndex = 1;

        for (auto &type : json["types"]) {
            Item subType = item;
            subType.Data = subIndex;

            for (auto it = type.begin(); it != type.end(); ++it) {
                Process_Item(it, subType);
            }

            item.Types[subIndex++] = subType;
        }

        ItemTypes[item.ID] = item;
    }

    closedir(itemDir);
}

const Block* Blocks::Get_Block(int type, int data) {
    return (data == 0) ?
        &BlockTypes[type] :
        &BlockTypes[type].Types[data];
}

const Item* Blocks::Get_Item(int type, int data) {
    return (data == 0) ?
        &ItemTypes[type] :
        &ItemTypes[type].Types[data];
}

const Block* Blocks::Get_Block_From_Name(std::string name) {
    for (auto const &block : BlockTypes) {
        if (Case_Insensitive_Cmp(name, block.second.Name)) {
            return &block.second;
        }
        else if (!block.second.Types.empty()) {
            for (auto const &subBlock : block.second.Types) {
                if (Case_Insensitive_Cmp(name, subBlock.second.Name)) {
                    return &subBlock.second;
                }
            }
        }
    }

    return nullptr;
}

const Item* Blocks::Get_Item_From_Name(std::string name) {
    for (auto const &item : ItemTypes) {
        if (Case_Insensitive_Cmp(name, item.second.Name)) {
            return &item.second;
        }
        else if (!item.second.Types.empty()) {
            for (auto const &subItem : item.second.Types) {
                if (Case_Insensitive_Cmp(name, subItem.second.Name)) {
                    return &subItem.second;
                }
            }
        }
    }

    return nullptr;
}

std::string Blocks::Get_Name(int type, int data) {
    if (BlockTypes.count(type)) {
        Block &block = BlockTypes[type];
        return (data == 0) ? block.Name : block.Types[data].Name;
    }
    else if (ItemTypes.count(type)) {
        Item &item = ItemTypes[type];
        return (data == 0) ? item.Name : item.Types[data].Name;
    }

    return "";
}

bool Blocks::Exists(int type, int data) {
    if (BlockTypes.count(type)) {
        if (data != 0) {
            return BlockTypes[type].Types.count(data);
        }

        return true;
    }

    else if (ItemTypes.count(type)) {
        if (data != 0) {
            return ItemTypes[type].Types.count(data);
        }

        return true;
    }

    return false;
}

Data Blocks::Mesh(const Block* block, glm::vec3 offset, float scale, Data data) {
    Data storage;
    Mesh(storage, block, offset, scale, data);
    return storage;
}

void Blocks::Mesh(Data &storage, const Block* block, glm::vec3 offset, float scale, Data data) {
    if (block->HasCustomData) {
        for (auto const &element : block->CustomData) {
            for (unsigned long i = 0; i < 6; i++) {
                for (unsigned long j = 0; j < 6; j++) {
                    Extend(storage, (element[i][j].first + offset) * scale);
                    Extend(storage, element[i][j].second);

                    for (float const &value : data) {
                        storage.push_back(value);
                    }
                }
            }
        }
    }
    else {
        for (unsigned long i = 0; i < 6; i++) {
            for (unsigned long j = 0; j < 6; j++) {
                Extend(storage, (vertices[i][j] + offset) * scale);

                if (block->MultiTextures) {
                    Extend(storage, tex_coords[i][j]);
                    storage.push_back(block->Textures[i]);
                }
                else if (block->HasTexture) {
                    Extend(storage, tex_coords[i][j]);
                    storage.push_back(block->Texture);
                }
            }
        }
    }
}
