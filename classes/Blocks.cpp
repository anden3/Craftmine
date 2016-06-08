#include "Blocks.h"

#include <dirent.h>
#include <sstream>
#include <fstream>
#include "json.hpp"

#include "../main.h"
#include "Interface.h"

bool iequals(const std::string &a, const std::string &b) {
    int sz = static_cast<int>(a.size());
    
    if (b.size() != sz) {
        return false;
    }
    
    for (unsigned int i = 0; i < sz; ++i) {
        if (tolower(a[i]) != tolower(b[i])) {
            return false;
        }
    }
    
    return true;
}

std::map<unsigned int, Block> BlockTypes;
std::map<unsigned int, Item> ItemTypes;

void Blocks::Init() {
    DIR* dir = opendir("BlockData");
    struct dirent* ent;
    
    std::vector<std::string> sides = {"left", "right", "down", "up", "back", "front"};
    
    while ((ent = readdir(dir)) != nullptr) {
        std::string name(ent->d_name);
        
        if (name.find(".json") != std::string::npos) {
            Block block;
            std::stringstream file_content;
            
            std::ifstream file("BlockData/" + name);
            file_content << file.rdbuf();
            file.close();
            
            nlohmann::json j;
            j << file_content;
            
            block.ID = j["id"];
            
            for (nlohmann::json::iterator it = j.begin(); it != j.end(); ++it) {
                if (it.key() == "name") { block.Name = it.value(); }
                else if (it.key() == "hardness") { block.Hardness = it.value(); }
                else if (it.key() == "transparent") { block.Transparent = it.value(); }
                else if (it.key() == "fullBlock") { block.FullBlock = it.value(); }
                else if (it.key() == "collision") { block.Collision = it.value(); }
                else if (it.key() == "targetable") { block.Targetable = it.value(); }
                else if (it.key() == "luminosity") { block.Luminosity = it.value(); }
                else if (it.key() == "sound") { block.Sound = it.value(); }
                
                else if (it.key() == "icon") {
                    block.HasIcon = true;
                    block.Icon = it.value();
                }
                
                else if (it.key() == "hitboxScale") {
                    for (int j = 0; j < 3; j++) {
                        block.Scale[j] = it.value()[j];
                    }
                }
                
                else if (it.key() == "hitboxOffset") {
                    for (int j = 0; j < 3; j++) {
                        block.ScaleOffset[j] = it.value()[j];
                    }
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
                            
                            int textureID = texture.is_array() ? texture[0] : texture;
                            glm::vec2 texStart(0);
                            glm::vec2 texEnd(1);
                            
                            if (texture.is_array()) {
                                texStart = glm::vec2(texture[1], texture[2]);
                                texEnd = glm::vec2(texture[3], texture[4]);
                            }
                            
                            for (int i = 0; i < 6; i++) {
                                data.push_back(std::make_pair(startPos + (endPos - startPos) * vertices[index][i],
                                               glm::vec3(texStart + (texEnd - texStart) * tex_coords[index][i], textureID)));
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
            
            if (j.count("types")) {
                int i = 0;
                
                for (auto &type : j["types"]) {
                    Block subType = block;
                    subType.Data = ++i;
                    
                    for (nlohmann::json::iterator it = type.begin(); it != type.end(); ++it) {
                        if (it.key() == "name") { subType.Name = it.value(); }
                        else if (it.key() == "hardness") { subType.Hardness = it.value(); }
                        else if (it.key() == "transparent") { subType.Transparent = it.value(); }
                        else if (it.key() == "fullBlock") { subType.FullBlock = it.value(); }
                        else if (it.key() == "collision") { subType.Collision = it.value(); }
                        else if (it.key() == "targetable") { subType.Targetable = it.value(); }
                        else if (it.key() == "luminosity") { subType.Luminosity = it.value(); }
                        else if (it.key() == "sound") { subType.Sound = it.value(); }
                        
                        else if (it.key() == "icon") {
                            subType.HasIcon = true;
                            subType.Icon = it.value();
                        }
                        
                        else if (it.key() == "hitboxScale") {
                            for (int j = 0; j < 3; j++) {
                                subType.Scale[j] = it.value()[j];
                            }
                        }
                        
                        else if (it.key() == "hitboxOffset") {
                            for (int j = 0; j < 3; j++) {
                                subType.ScaleOffset[j] = it.value()[j];
                            }
                        }
                        
                        else if (it.key() == "texture") {
                            subType.HasTexture = true;
                            subType.Texture = it.value();
                        }
                        
                        else if (it.key() == "multiTexture") {
                            subType.MultiTextures = true;
                            
                            for (auto const &side : it.value()) {
                                subType.Textures.push_back(side);
                            }
                        }
                        
                        else if (it.key() == "elements") {
                            subType.HasCustomData = true;
                            
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
                                
                                subType.CustomData.push_back({});
                                int index = 0;
                                
                                for (auto const &texture : element["texCoords"]) {
                                    std::vector<std::pair<glm::vec3, glm::vec3>> data;
                                    
                                    int textureID = texture.is_array() ? texture[0] : texture;
                                    glm::vec2 texStart(0);
                                    glm::vec2 texEnd(1);
                                    
                                    if (texture.is_array()) {
                                        texStart = glm::vec2(texture[1], texture[3]);
                                        texEnd = glm::vec2(texture[2], texture[4]);
                                    }
                                    
                                    for (int i = 0; i < 6; i++) {
                                        data.push_back(std::make_pair(startPos + (endPos - startPos) * vertices[index][i],
                                                       glm::vec3(texStart + (texEnd - texStart) * tex_coords[index][i], textureID)));
                                    }
                                    
                                    subType.CustomData.back().push_back(data);
                                    ++index;
                                }
                            }
                            
                            if (subType.Scale == glm::vec3(1)) {
                                subType.Scale = largestCoord - smallestCoord;
                            }
                            
                            if (subType.ScaleOffset == glm::vec3(0)) {
                                subType.ScaleOffset = smallestCoord;
                            }
                        }
                    }
                    
                    block.Types[i] = subType;
                }
            }
            
            BlockTypes[block.ID] = block;
        }
    }
    
    closedir(dir);
    
    DIR* itemDir = opendir("/Users/mac/Documents/Craftmine/Craftmine/ItemData");
    struct dirent* itemEnt;
    
    while ((itemEnt = readdir(itemDir)) != nullptr) {
        std::string name(itemEnt->d_name);
        
        if (name.find(".json") != std::string::npos) {
            Item item;
            std::stringstream file_content;
            
            std::ifstream file("/Users/mac/Documents/Craftmine/Craftmine/ItemData/" + name);
            file_content << file.rdbuf();
            file.close();
            
            nlohmann::json j;
            j << file_content;
            
            for (nlohmann::json::iterator it = j.begin(); it != j.end(); ++it) {
                if (it.key() == "id") { item.ID = it.value(); }
                else if (it.key() == "name") { item.Name = it.value(); }
                else if (it.key() == "icon") { item.Icon = it.value(); }
            }
            
            if (j.count("types")) {
                int i = 0;
                
                for (auto &type : j["types"]) {
                    Item subType = item;
                    subType.Data = ++i;
                    
                    for (nlohmann::json::iterator it = type.begin(); it != type.end(); ++it) {
                        if (it.key() == "name") { subType.Name = it.value(); }
                        if (it.key() == "icon") { subType.Icon = it.value(); }
                    }
                    
                    item.Types[i] = subType;
                }
            }
            
            ItemTypes[item.ID] = item;
        }
    }
    
    closedir(itemDir);
}

const Block* Blocks::Get_Block(unsigned int type, int data) {
    return (data == 0) ? &BlockTypes[type] : &BlockTypes[type].Types[data];
}

const Item* Blocks::Get_Item(unsigned int type, int data) {
    return (data == 0) ? &ItemTypes[type] : &ItemTypes[type].Types[data];
}

const Block* Blocks::Get_Block_From_Name(std::string name) {
    for (auto const &block : BlockTypes) {
        if (iequals(name, block.second.Name)) {
            return &block.second;
        }
        else if (!block.second.Types.empty()) {
            for (auto const &subBlock : block.second.Types) {
                if (iequals(name, subBlock.second.Name)) {
                    return &subBlock.second;
                }
            }
        }
    }
    
    return nullptr;
}

const Item* Blocks::Get_Item_From_Name(std::string name) {
    for (auto const &item : ItemTypes) {
        if (iequals(name, item.second.Name)) {
            return &item.second;
        }
        else if (!item.second.Types.empty()) {
            for (auto const &subItem : item.second.Types) {
                if (iequals(name, subItem.second.Name)) {
                    return &subItem.second;
                }
            }
        }
    }
    
    return nullptr;
}

std::string Blocks::Get_Name(unsigned int type, int data) {
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

bool Blocks::Exists(unsigned int type, int data) {
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
            for (int i = 0; i < 6; i++) {
                for (int j = 0; j < 6; j++) {
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
        for (int i = 0; i < 6; i++) {
            for (int j = 0; j < 6; j++) {
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