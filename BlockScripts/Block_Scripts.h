#pragma once

#include <map>
#include <string>
#include <functional>

extern std::map<std::string, std::function<void()>> BlockRightClick;
extern std::map<std::string, std::function<void()>> BlockUpdate;
extern std::map<std::string, std::function<void()>> BlockClose;

void Init_Block_Scripts();
