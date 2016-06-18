#pragma once

#include <string>

namespace System {
    std::string GetVirtualMemoryUsage();
    std::string GetPhysicalMemoryUsage();
    std::string GetVRAMUsage();

    double GetCPUUsage();
}

template <typename I>
std::string FormatOutput(I usage) {
    std::string units[4] = { "B", "KB", "MB", "GB" };
    int unitIndex = 0;

    while (usage >= 1024) {
        usage /= 1024;
        unitIndex++;
    }

    return std::string(std::to_string(usage) + " " + units[unitIndex]);
}

void InitSystem();
