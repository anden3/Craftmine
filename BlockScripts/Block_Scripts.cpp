#include "Block_Scripts.h"

#include "Furnace.h"

std::map<std::string, std::function<void()>> BlockRightClick = {
	{"Furnace", Furnace::Right_Click},
};

std::map<std::string, std::function<void()>> BlockUpdate = {
	{"Furnace", Furnace::Update},
};

std::map<std::string, std::function<void()>> BlockClose = {
	{"Furnace", Furnace::Close},
};

void Init_Block_Scripts() {
	Furnace::Init();
};
