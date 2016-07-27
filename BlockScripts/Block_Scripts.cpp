#include "Block_Scripts.h"

#include "Furnace.h"

std::map<std::string, std::function<void()>> BlockFunctions = {
	{"Furnace", Furnace::Right_Click},
};
void Init_Block_Scripts() {
	Furnace::Init();
};
