#include "Worlds.h"

#include "main.h"
#include "Chunk.h"
#include "Camera.h"
#include "Player.h"
#include "Network.h"
#include "Interface.h"
#include "Inventory.h"

#include <fstream>
#include <sstream>

#include <dirent.h>
#include <json.hpp>

#include <boost/filesystem.hpp>

static std::map<std::string, int> WorldList;

int Hex_To_Dec(std::string hex) {
    int result;
    std::stringstream ss;
    ss << std::hex << hex;
    ss >> result;
    return result;
}

int Hex_To_Dec(const char hex) {
    return Hex_To_Dec(std::string(1, hex));
}

std::string Dec_To_Hex(int dec) {
    std::stringstream ss;
    ss << std::hex << dec;
    return ss.str();
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

void Worlds::Save_World() {
    nlohmann::json playerData;

    playerData["Position"] = {
        player.WorldPos.x, player.WorldPos.y, player.WorldPos.z
    };

    playerData["Yaw"]   = Cam.Yaw;
    playerData["Pitch"] = Cam.Pitch;

    if (Inventory::HoldingStack.Type) {
        Inventory::Add_Stack(&Inventory::HoldingStack);
        Inventory::HoldingStack.Clear();
    }

    Inventory::Save(playerData, "Inventory");
    Inventory::Save(playerData, "Crafting");

    if (Inventory::CraftingOutput->Contents.Type) {
        playerData["Storage"]["CraftingOutput"] = {
            Inventory::CraftingOutput->Contents.Type,
            Inventory::CraftingOutput->Contents.Data,
            Inventory::CraftingOutput->Contents.Size
        };
    }

    if (Multiplayer) {
        playerData["event"] = "save";
        Network::Send(playerData.dump());
        Network::Update();
    }
    else {
        std::ofstream dataFile("Worlds/" + WORLD_NAME + "/Player.json", std::ofstream::trunc);
        dataFile << playerData;
    }
}

void Worlds::Load_World(int seed) {
    if (Multiplayer) {
        Inventory::Clear();
        ChunkMap.clear();
    }

    else {
        std::ifstream dataFile("Worlds/" + WORLD_NAME + "/Player.json");

        if (dataFile.good()) {
            std::stringstream ss;
            ss << dataFile.rdbuf();
            player.Load_Data(ss.str());
        }
        else {
            Inventory::Clear();
            ChunkMap.clear();
        }
    }

    Chunks::Seed(seed);
	player.Update(true);
}

std::vector<World> Worlds::Get_Worlds() {
    std::vector<World> worlds;

    DIR* worldDir = opendir("Worlds");
    struct dirent* worldEnt;

    while ((worldEnt = readdir(worldDir)) != nullptr) {
        std::string dirName(worldEnt->d_name);

        if (dirName == "." || dirName == ".." || dirName == ".DS_Store") {
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

    std::stringstream buffer;
    buffer << file.rdbuf();

    for (auto const &b : Split(buffer.str(), '-')) {
        glm::vec3 blockPos(
            Hex_To_Dec(b[0]), Hex_To_Dec(b[1]), Hex_To_Dec(b[2])
        );

        std::pair<int, int> blockType = {0, 0};

        if (b.find(':') > 3) {
            blockType.first = Hex_To_Dec(b.substr(3, (b.find(':') - 3)));
        }

        if (b.back() != ':') {
            blockType.second = Hex_To_Dec(b.substr(b.find(':')));
        }

        changedBlocks[blockPos] = blockType;
    }

    return changedBlocks;
}