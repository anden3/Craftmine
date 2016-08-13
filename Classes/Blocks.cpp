#include "Blocks.h"

#include <dirent.h>
#include <sstream>
#include <fstream>
#include "json.hpp"

#include "main.h"
#include "Interface.h"

#include "../BlockScripts/Block_Scripts.h"

#define BLOCK_LAMBDA(_v) [](Block &b, JSONValue val) { b._v = val; }
#define STRING_LAMBDA(_v) [](Block &b, JSONValue val) { b._v = val.get<std::string>(); }
#define VECTOR_LAMBDA(_v) [](Block &b, JSONValue val) { \
    for (unsigned long j = 0; j < 3; j++) \
        b._v[static_cast<int>(j)] = val[j]; \
}

typedef nlohmann::basic_json<
    std::map, std::vector, std::basic_string<
        char, std::char_traits<char>, std::allocator<char>
    >, bool, long long, double, std::allocator
> JSONValue;

static std::map<std::string, std::function<void(Block &b, JSONValue val)>> lambdas = {
    {"id",                  BLOCK_LAMBDA(ID)                 },
	{"isTool",              BLOCK_LAMBDA(IsTool)             },
    {"hardness",            BLOCK_LAMBDA(Hardness)           },
	{"isMBRoot",            BLOCK_LAMBDA(IsMBRoot)           },
    {"collision",           BLOCK_LAMBDA(Collision)          },
	{"fullBlock",           BLOCK_LAMBDA(FullBlock)          },
	{"placeable",           BLOCK_LAMBDA(Placeable)          },
    {"durability",          BLOCK_LAMBDA(Durability)         },
    {"luminosity",          BLOCK_LAMBDA(Luminosity)         },
    {"multiBlock",          BLOCK_LAMBDA(MultiBlock)         },
	{"targetable",          BLOCK_LAMBDA(Targetable)         },
	{"miningLevel",         BLOCK_LAMBDA(MiningLevel)        },
	{"miningSpeed",         BLOCK_LAMBDA(MiningSpeed)        },
	{"transparent",         BLOCK_LAMBDA(Transparent)		 },
	{"craftingYield",       BLOCK_LAMBDA(CraftingYield)      },
	{"requiredMiningLevel", BLOCK_LAMBDA(RequiredMiningLevel)},

	{"name",                STRING_LAMBDA(Name)              },
	{"sound",               STRING_LAMBDA(Sound)             },
	{"material",            STRING_LAMBDA(Material)          },
	{"effectiveMat",        STRING_LAMBDA(EffectiveMaterial) },

    {"mBOffset",            VECTOR_LAMBDA(MBOffset)          },
    {"hitboxScale",         VECTOR_LAMBDA(Scale)             },
    {"hitboxOffset",        VECTOR_LAMBDA(ScaleOffset)       }
};

static std::map<int, Block> BlockTypes;
static std::map<std::string, Block> BlockNames;

bool Case_Insensitive_Cmp(const std::string &a, const std::string &b) {
    size_t sz = a.size();

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

static nlohmann::json Parse_JSON(std::string path) {
    nlohmann::json json;
    std::ifstream file(path);
    json << file;
    return json;
}

std::regex Parse_Recipe(std::string recipe) {
	std::vector<std::string> setNums;

	std::string rgx = "";
	std::string setNum = "";
	std::string capture = "";

	bool set = false;
	bool capturing = false;
	bool checkNext = false;
	bool setAcceptZero = false;
	bool ignoreNextSpace = false;

	for (const char &c : recipe) {
		if (c == '(') {
			capture.clear();
			capturing = true;
		}

		else if (c == '%') {
			rgx += "(0,)*";
			ignoreNextSpace = true;
		}

		else if (capturing) {
			if (c == ')') {
				capturing = false;
				checkNext = true;
			}
			else {
				capture += (c == ' ') ? ',' : c;
			}
		}

		else if (checkNext) {
			checkNext = false;

			if (isdigit(c)) {
				for (int i = 0; i < (c - 48); i++) {
					rgx += capture + ",";
				}

				rgx.pop_back();
			}

			else if (c == '[') {
				set = true;
			}
		}

		else if (set) {
			if (c == ']') {
				set = false;

				setNums.push_back(setNum);
				setNum.clear();

				if (setAcceptZero) {
					rgx += "(";
				}

				for (auto const &num : setNums) {
					rgx += "(" + capture + ",){" + num + "}|";
				}

				rgx.pop_back();

				if (setAcceptZero) {
					rgx += ")?";
				}

				ignoreNextSpace = true;
				setAcceptZero = false;
			}

			else if (c == '0') {
				setAcceptZero = true;
			}

			else if (isdigit(c)) {
				setNum += c;
			}

			else if (c == ',' && setNum != "") {
				setNums.push_back(setNum);
				setNum = "";
			}
		}

		else {
			if (c != ' ') {
				rgx += c;
			}
			else if (!ignoreNextSpace) {
				rgx += ",";
			}

			ignoreNextSpace = false;
		}
	}

	if (rgx.back() != ',' && rgx.back() != '*') {
		rgx += ",";
	}

	return std::regex("^" + rgx + "$");
}

void Process_Block(nlohmann::json::iterator it, Block &block) {
    if (lambdas.count(it.key())) {
        lambdas[it.key()](block, it.value());
    }

    else if (it.key() == "icon") {
        block.HasIcon = true;
        block.Icon = it.value();
    }

	else if (it.key() == "recipe") {
		block.Craftable = true;
		block.Recipe = Parse_Recipe(it.value());
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

void Blocks::Init() {
    DIR* blockDir = opendir("BlockData");
    struct dirent* blockEnt;

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

        if (json.count("types")) {
			int subIndex = 1;

			for (auto &type : json["types"]) {
				Block subType = block;
				subType.Data = subIndex;

				for (auto it = type.begin(); it != type.end(); ++it) {
					Process_Block(it, subType);
				}

				if (BlockFunctions.count(subType.Name)) {
					subType.Interactable = true;
					subType.RightClickFunction = BlockFunctions[subType.Name];
				}

				block.Types[subIndex++] = subType;
				BlockNames[subType.Name] = subType;
			}
        }

		if (BlockFunctions.count(block.Name)) {
			block.Interactable = true;
			block.RightClickFunction = BlockFunctions[block.Name];
		}

        BlockTypes[block.ID] = block;
        BlockNames[block.Name] = block;
    }

    closedir(blockDir);
}

const Block* Blocks::Get_Block(int type, int data) {
    return (data == 0) ?
        &BlockTypes[type] :
        &BlockTypes[type].Types[data];
}

const Block* Blocks::Get_Block(std::string name) {
    for (auto const &block : BlockNames) {
        if (Case_Insensitive_Cmp(name, block.first)) {
            return &block.second;
        }
    }
    return nullptr;
}

std::string Blocks::Get_Name(int type, int data) {
    if (BlockTypes.count(type)) {
        Block &block = BlockTypes[type];
        return (data == 0) ? block.Name : block.Types[data].Name;
    }
    return "";
}

bool Blocks::Exists(int type, int data) {
    if (BlockTypes.count(type)) {
        if (data == 0) {
			return true;
        }

		return BlockTypes[type].Types.count(data) > 0;
    }

    return false;
}

bool Blocks::Exists(std::string name) {
    return BlockNames.count(name) > 0;
}

const Block* Blocks::Check_Crafting(std::string grid) {
	for (auto const &block : BlockNames) {
		if (block.second.Craftable) {
			if (std::regex_match(grid.cbegin(), grid.cend(), block.second.Recipe)) {
				return &block.second;
			}
		}
	}

	return nullptr;
}

Data Blocks::Mesh(const Block* block, glm::vec3 offset, float scale, Data data) {
    Data storage;
    Mesh(storage, block, offset, scale, data);
    return storage;
}

void Blocks::Mesh(Data &storage, const Block* block, glm::vec3 offset, float scale, Data data, bool checkMulti) {
    if (checkMulti && block->MultiBlock) {
        for (auto const &element : block->Types) {
            Mesh(storage, &element.second, offset + static_cast<glm::vec3>(element.second.MBOffset), scale, data, false);
        }

        return;
    }

    else if (!block->Placeable) {
        for (unsigned long i = 0; i < 6; i++) {
            Extend(storage, (vertices[UP][i] + offset) * scale);
            Extend(storage, tex_coords[UP][i]);
            storage.push_back(static_cast<float>(block->Icon));
        }
    }

    else if (block->HasCustomData) {
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
                Extend(storage, tex_coords[i][j]);

                if (block->MultiTextures) {
                    storage.push_back(static_cast<float>(block->Textures[i]));
                }
                else if (block->HasTexture) {
                    storage.push_back(static_cast<float>(block->Texture));
                }
            }
        }
    }
}
