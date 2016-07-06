#include "Worlds.h"

#include "main.h"
#include "Chunk.h"
#include "Player.h"

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

    std::ofstream propFile("Worlds/" + name + "/properties.json", std::fstream::out);
    propFile << properties;
}

void Worlds::Delete_World(std::string name) {
    boost::filesystem::remove_all("Worlds/" + name);
}

void Worlds::Load_World(int seed) {
    ChunkMap.clear();
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

        std::ifstream file("Worlds/" + dirName + "/properties.json");
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