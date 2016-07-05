#include "Worlds.h"

#include "main.h"
#include "Chunk.h"

#include <fstream>
#include <sstream>

#include <dirent.h>
#include <json.hpp>

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

void Worlds::Get_Worlds() {
    DIR* worldDir = opendir("Worlds");
    struct dirent* worldEnt;

    while ((worldEnt = readdir(worldDir)) != nullptr) {
        std::string fileName(worldEnt->d_name);

        if (fileName == "." || fileName == "..") {
            continue;
        }
    }
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