#include "System.h"

#include <Windows.h>
#include <Psapi.h>

#include <GLEW/glew.h>

static ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
static int numProcessors;
static HANDLE self;

static int totalVRAM;

std::string units[4] = { "B", "KB", "MB", "GB" };

bool initiated = false;

PROCESS_MEMORY_COUNTERS_EX GetMemoryInfo() {
	if (!initiated) InitSystem();

	PROCESS_MEMORY_COUNTERS_EX info;
	GetProcessMemoryInfo(self, (PROCESS_MEMORY_COUNTERS*)&info, sizeof(info));

	return info;
}

std::string FormatOutput(SIZE_T usage) {
	int unitIndex = 0;

	while (usage >= 1024) {
		usage /= 1024;
		unitIndex++;
	}

	return std::string(std::to_string(usage) + " " + units[unitIndex]);
}

std::string System::GetVirtualMemoryUsage() {
	return FormatOutput(GetMemoryInfo().PrivateUsage);
}

std::string System::GetPhysicalMemoryUsage() {
	return FormatOutput(GetMemoryInfo().WorkingSetSize);
}

std::string System::GetVRAMUsage() {
	int availableMemory;
	glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &availableMemory);

	return FormatOutput((SIZE_T) ((totalVRAM - availableMemory) * 1024));
}

double System::GetCPUUsage() {
	if (!initiated) {
		InitSystem();
	}

	FILETIME ftime, fsys, fuser;
	ULARGE_INTEGER now, sys, user;

	GetSystemTimeAsFileTime(&ftime);
	memcpy(&now, &ftime, sizeof(FILETIME));

	GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
	memcpy(&sys, &fsys, sizeof(FILETIME));
	memcpy(&user, &fuser, sizeof(FILETIME));

	double percent = (double) ((sys.QuadPart - lastSysCPU.QuadPart) + (user.QuadPart - lastUserCPU.QuadPart));
	percent /= (double) (now.QuadPart - lastCPU.QuadPart);
	percent /= numProcessors;
	percent *= 100.0;

	lastCPU = now;
	lastSysCPU = sys;
	lastUserCPU = user;

	return percent;
}

void InitSystem() {
	initiated = true;

	SYSTEM_INFO sysInfo;
	FILETIME ftime, fsys, fuser;

	GetSystemInfo(&sysInfo);
	numProcessors = sysInfo.dwNumberOfProcessors;

	GetSystemTimeAsFileTime(&ftime);
	memcpy(&lastCPU, &ftime, sizeof(FILETIME));

	self = GetCurrentProcess();
	GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
	memcpy(&lastSysCPU, &fsys, sizeof(FILETIME));
	memcpy(&lastUserCPU, &fuser, sizeof(FILETIME));

	glGetIntegerv(GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &totalVRAM);
}