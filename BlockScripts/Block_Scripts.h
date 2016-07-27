#pragma once

#include <map>
#include <string>
#include <functional>

extern std::map<std::string, std::function<void()>> BlockFunctions;

void Init_Block_Scripts();
