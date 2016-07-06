#include "Worlds.h"

#include "main.h"
#include "Chunk.h"
#include "Camera.h"
#include "Player.h"
#include "Inventory.h"

#include <fstream>
#include <sstream>

#include <dirent.h>
#include <json.hpp>

#include <boost/filesystem.hpp>

static std::map<std::string, int> WorldList;

// Only works for uppercase.
constexpr int Hex_To_Dec(char hex) {
    if (hex >= 'A') {  return hex - 'A' + 10; }
    return hex - '0';
}

// Only outputs uppercase hex.
constexpr char Dec_To_Hex(int val) {
    if (val >= 10) { return static_cast<char>(val - 10 + 'A'); }
    return static_cast<char>(val + '0');
}

int Worlds::Get_Seed(std::string name) {
    if (WorldList.count(name)) {
        return WorldList[name];
    }
    return 0;
}

std::string Worlds::Get_Name(int seed) {
    for (auto const &world : WorldList) {
        if (world.second == seed) {
            return world.first;
        }
    }

    return "";
}

void Worlds::Create_World(std::string name, int seed) {
    boost::filesystem::path newWorld("Worlds/" + name);
    boost::filesystem::create_directory(newWorld);

    newWorld += "/Chunks";
    boost::filesystem::create_directory(newWorld);

    nlohmann::json properties;
    properties["name"] = name;
    properties["seed"] = seed;

    std::ofstream propFile("Worlds/" + name + "/Properties.json");
    propFile << properties;
}

void Worlds::Delete_World(std::string name) {
    boost::filesystem::remove_all("Worlds/" + name);
}

void Add_Storage_Data(nlohmann::json &dest, std::string type, std::vector<Stack> &storage) {
    int currentSlot = 0;

    for (auto const &item : storage) {
        if (!item.Type) {
            ++currentSlot;
            continue;
        }

        if (item.Data) {
            dest["Storage"][type][std::to_string(currentSlot)] = { item.Type, item.Data, item.Size };
        }
        else {
            dest["Storage"][type][std::to_string(currentSlot)] = { item.Type, item.Size };
        }

        ++currentSlot;
    }
}

void Load_Storage_Data(nlohmann::json &src, std::string type, std::vector<Stack> &storage) {
    if (!src.count("Storage") || !src["Storage"].count(type)) {
        return;
    }

    for (auto it = src["Storage"][type].begin(); it != src["Storage"][type].end(); ++it) {
        if (it.value().size() == 3) {
            storage[std::stoul(it.key())] = {
                it.value()[0], it.value()[1], it.value()[2]
            };
        }
        else {
            storage[std::stoul(it.key())] = Stack(
                it.value()[0].get<int>(), it.value()[1]
            );
        }
    }
}

void Worlds::Save_World() {
    nlohmann::json playerData;

    playerData["Position"] = {
        player.WorldPos.x, player.WorldPos.y, player.WorldPos.z
    };

    playerData["Yaw"]   = Cam.Yaw;
    playerData["Pitch"] = Cam.Pitch;

    if (inventory.HoldingStack.Type) {
        inventory.Add_Stack(inventory.HoldingStack);
        inventory.HoldingStack.Clear();
    }

    Add_Storage_Data(playerData, "Inventory", inventory.Inv);
    Add_Storage_Data(playerData, "Toolbar", inventory.Toolbar);
    Add_Storage_Data(playerData, "Crafting", inventory.Craft);

    if (inventory.CraftingOutput.Type) {
        playerData["Storage"]["CraftingOutput"] = {
            inventory.CraftingOutput.Type,
            inventory.CraftingOutput.Data,
            inventory.CraftingOutput.Size
        };
    }

    std::ofstream dataFile("Worlds/" + WORLD_NAME + "/Player.json", std::ofstream::trunc);
    dataFile << playerData;
}

void Worlds::Load_World(int seed) {
    inventory.Clear();
    ChunkMap.clear();

    std::ifstream dataFile("Worlds/" + WORLD_NAME + "/Player.json");

    if (dataFile.good()) {
        nlohmann::json playerData;
        playerData << dataFile;

        Cam.Yaw   = playerData["Yaw"];
        Cam.Pitch = playerData["Pitch"];
        Cam.UpdateCameraVectors();

        player.Teleport(glm::vec3(
            playerData["Position"][0], playerData["Position"][1], playerData["Position"][2]
        ));

        Load_Storage_Data(playerData, "Inventory", inventory.Inv);
        Load_Storage_Data(playerData, "Toolbar", inventory.Toolbar);
        Load_Storage_Data(playerData, "Crafting", inventory.Craft);

        inventory.Mesh();
    }

    Chunks::Seed(seed);
    player.Queue_Chunks(true);
}

std::vector<World> Worlds::Get_Worlds() {
    std::vector<World> worlds;

    DIR* worldDir = opendir("Worlds");
    struct dirent* worldEnt;

    while ((worldEnt = readdir(worldDir)) != nullptr) {
        std::string dirName(worldEnt->d_name);

        if (dirName == "." || dirName == "..") {
            continue;
        }

        nlohmann::json json;
        std::stringstream file_content;

        std::ifstream file("Worlds/" + dirName + "/Properties.json");
        file_content << file.rdbuf();
        file.close();

        json << file_content;
        worlds.emplace_back(World(json["name"], json["seed"]));
        WorldList[json["name"]] = json["seed"];
    }

    return worlds;
}

void Worlds::Save_Chunk(std::string world, glm::vec3 chunkPos) {
    if (!ChangedBlocks.count(chunkPos)) {
        return;
    }

    glm::ivec3 pos = static_cast<glm::ivec3>(chunkPos);

    std::string fileName(std::to_string(pos.x) + "," + std::to_string(pos.y) + "," + std::to_string(pos.z) + ".chunk");
    std::ofstream file("Worlds/" + world + "/Chunks/" + fileName, std::ofstream::trunc);

    for (auto const &block : ChangedBlocks[chunkPos]) {
        glm::ivec3 blockPos = static_cast<glm::ivec3>(block.first);
        file << Dec_To_Hex(blockPos.x) << Dec_To_Hex(blockPos.y) << Dec_To_Hex(blockPos.z);

        if (block.second.first > 0) {
            file << std::hex << block.second.first;
        }

        file << ':';

        if (block.second.second > 0) {
            file << std::hex << block.second.second;
        }

        file << '-';
    }
}

std::map<glm::vec3, std::pair<int, int>, VectorComparator> Worlds::Load_Chunk(std::string world, glm::ivec3 pos) {
    std::map<glm::vec3, std::pair<int, int>, VectorComparator> changedBlocks;

    std::string fileName(std::to_string(pos.x) + "," + std::to_string(pos.y) + "," + std::to_string(pos.z) + ".chunk");
    std::ifstream file("Worlds/" + world + "/Chunks/" + fileName);

    if (!file.good()) {
        return changedBlocks;
    }

    char c;
    int index = 0;
    bool dataFound = false;

    glm::ivec3 blockPos = {0, 0, 0};
    std::pair<int, int> blockType = {0, 0};

    std::vector<int> idVal;

    while (file.get(c)) {
        if (index < 3) {
            blockPos[index] = Hex_To_Dec(c);
        }
        else if (!dataFound && c != ':') {
            for (int &val : idVal) { val *= 16; }
            idVal.push_back(Hex_To_Dec(c));
        }
        else if (c == ':') {
            for (int const &val : idVal) { blockType.first += val; }
            dataFound = true;
        }
        else if (c != '-') {
            if (blockType.second > 0) { continue; }

            blockType.second = Hex_To_Dec(c);
            char nextChar = static_cast<char>(file.peek());

            if (nextChar != '-') {
                blockType.second = blockType.second * 16 + Hex_To_Dec(nextChar);
            }
        }
        else {
            changedBlocks[static_cast<glm::vec3>(blockPos)] = blockType;
            blockType = {0, 0};
            idVal.clear();
            dataFound = false;
            index = -1;
        }

        ++index;
    }

    return changedBlocks;
}