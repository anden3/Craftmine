#include "Blocks.h"

#include <sstream>
#include <fstream>
#include <dirent.h>

#include "json.hpp"

bool iequals(const std::string &a, const std::string &b) {
    unsigned long sz = a.size();
    
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

void Blocks::Init() {
    DIR *dir = opendir("blockData");
    struct dirent *ent;
    
    std::vector<std::string> sides = {"left", "right", "down", "up", "back", "front"};
    
    while ((ent = readdir(dir)) != nullptr) {
        std::string name(ent->d_name);
        
        if (name.find(".json") != std::string::npos) {
            Block block;
            
            std::stringstream file_content;
            
            std::ifstream file("blockData/" + name);
            file_content << file.rdbuf();
            file.close();
            
            nlohmann::json j;
            j << file_content;
            
            block.ID = j["id"];
            
            for (nlohmann::json::iterator it = j.begin(); it != j.end(); ++it) {
                if (it.key() == "name") { block.Name = it.value(); }
                else if (it.key() == "hardness") { block.Hardness = it.value(); }
                else if (it.key() == "transparent") { block.Transparent = it.value(); }
                else if (it.key() == "noCollision") { block.Collision = !it.value(); }
                else if (it.key() == "targetable") { block.Targetable = it.value(); }
                else if (it.key() == "luminosity") { block.Luminosity = it.value(); }
                else if (it.key() == "sound") { block.Sound = it.value(); }
                
                else if (it.key() == "icon") {
                    block.HasIcon = true;
                    block.Icon = glm::vec2(it.value()[0], it.value()[1]);
                }
                
                else if (it.key() == "texture") {
                    block.HasTexture = true;
                    block.Texture = glm::vec2(it.value()[0], it.value()[1]);
                }
                
                else if (it.key() == "multiTexture") {
                    block.MultiTextures = true;
                    
                    std::map<std::string, glm::vec2> textureBuffer;
                    
                    for (nlohmann::json::iterator at = it.value().begin(); at != it.value().end(); ++at) {
                        textureBuffer[at.key()] = glm::vec2(at.value()[0], at.value()[1]);
                    }
                    
                    for (auto const &side : sides) {
                        block.Textures.push_back(textureBuffer[side]);
                    }
                }
                
                else if (it.key() == "texCoords") {
                    block.CustomTexCoords = true;
                    std::map<std::string, glm::vec4> textureBuffer;
                    
                    for (nlohmann::json::iterator at = it.value().begin(); at != it.value().end(); ++at) {
                        textureBuffer[at.key()] = glm::vec4(at.value()[0], at.value()[1], at.value()[2], at.value()[3]);
                    }
                    
                    for (auto const &side : sides) {
                        block.TexCoords.push_back(std::vector<glm::vec2> {textureBuffer[side].xy(), textureBuffer[side].zw()});
                    }
                }
                
                else if (it.key() == "vertices") {
                    block.CustomVertices = true;
                    std::map<std::string, std::pair<glm::vec3, glm::vec3>> textureBuffer;
                    
                    for (nlohmann::json::iterator at = it.value().begin(); at != it.value().end(); ++at) {
                        textureBuffer[at.key()] = std::make_pair(glm::vec3(at.value()[0], at.value()[1], at.value()[2]), glm::vec3(at.value()[3], at.value()[4], at.value()[5]));
                    }
                    
                    for (auto const &side : sides) {
                        block.Vertices.push_back(std::vector<glm::vec3> {textureBuffer[side].first, textureBuffer[side].second});
                    }
                }
            }
            
            if (j.count("types")) {
                for (nlohmann::json::iterator at = j["types"].begin(); at != j["types"].end(); ++at) {
                    Block subType = block;
                    int index = std::stoi(at.key());
                    subType.Data = index;
                    
                    for (nlohmann::json::iterator it = at.value().begin(); it != at.value().end(); ++it) {
                        if (it.key() == "name") { subType.Name = it.value(); }
                        else if (it.key() == "hardness") { subType.Hardness = it.value(); }
                        else if (it.key() == "transparent") { subType.Transparent = it.value(); }
                        else if (it.key() == "noCollision") { subType.Collision = !it.value(); }
                        else if (it.key() == "targetable") { subType.Targetable = it.value(); }
                        else if (it.key() == "luminosity") { subType.Luminosity = it.value(); }
                        else if (it.key() == "sound") { subType.Sound = it.value(); }
                        
                        else if (it.key() == "icon") {
                            subType.HasIcon = true;
                            subType.Icon = glm::vec2(it.value()[0], it.value()[1]);
                        }
                        
                        else if (it.key() == "texture") {
                            subType.HasTexture = true;
                            subType.Texture = glm::vec2(it.value()[0], it.value()[1]);
                        }
                        
                        else if (it.key() == "multiTexture") {
                            subType.MultiTextures = true;
                            
                            std::map<std::string, glm::vec2> textureBuffer;
                            
                            for (nlohmann::json::iterator at = it.value().begin(); at != it.value().end(); ++at) {
                                textureBuffer[at.key()] = glm::vec2(at.value()[0], at.value()[1]);
                            }
                            
                            for (auto const &side : sides) {
                                subType.Textures.push_back(textureBuffer[side]);
                            }
                        }
                        
                        else if (it.key() == "texCoords") {
                            subType.CustomTexCoords = true;
                            std::map<std::string, glm::vec4> textureBuffer;
                            
                            for (nlohmann::json::iterator at = it.value().begin(); at != it.value().end(); ++at) {
                                textureBuffer[at.key()] = glm::vec4(at.value()[0], at.value()[1], at.value()[2], at.value()[3]);
                            }
                            
                            for (auto const &side : sides) {
                                subType.TexCoords.push_back(std::vector<glm::vec2> {textureBuffer[side].xy(), textureBuffer[side].zw()});
                            }
                        }
                        
                        else if (it.key() == "vertices") {
                            subType.CustomVertices = true;
                            std::map<std::string, std::pair<glm::vec3, glm::vec3>> textureBuffer;
                            
                            for (nlohmann::json::iterator at = it.value().begin(); at != it.value().end(); ++at) {
                                textureBuffer[at.key()] = std::make_pair(glm::vec3(at.value()[0], at.value()[1], at.value()[2]), glm::vec3(at.value()[3], at.value()[4], at.value()[5]));
                            }
                            
                            for (auto const &side : sides) {
                                subType.Vertices.push_back(std::vector<glm::vec3> {textureBuffer[side].first, textureBuffer[side].second});
                            }
                        }
                    }
                    
                    block.Types[index] = subType;
                }
            }
            
            BlockTypes[j["id"]] = block;
        }
    }
    
    closedir(dir);
}

const Block* Blocks::Get_Block(unsigned int type, int data) {
    return (data == 0) ? &BlockTypes[type] : &BlockTypes[type].Types[data];
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

std::string Blocks::Get_Name(unsigned int type, int data) {
    Block &block = BlockTypes[type];
    if (data == 0) {
        return block.Name;
    }
    else {
        return block.Types[data].Name;
    }
}

bool Blocks::Exists(unsigned int type, int data) {
    if (BlockTypes.count(type)) {
        if (data != 0) {
            return BlockTypes[type].Types.count(data);
        }
        
        return true;
    }
    
    return false;
}