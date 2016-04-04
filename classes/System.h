#pragma once

#include <string>

namespace System {
	std::string GetVirtualMemoryUsage();
	std::string GetPhysicalMemoryUsage();
	std::string GetVRAMUsage();

	double GetCPUUsage();
}

void InitSystem();