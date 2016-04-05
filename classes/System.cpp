#include "System.h"

#include <GL/glew.h>

static int totalVRAM;
bool initiated = false;

std::string units[4] = { "B", "KB", "MB", "GB" };

std::string FormatOutput(int usage) {
    int unitIndex = 0;
    
    while (usage >= 1024) {
        usage /= 1024;
        unitIndex++;
    }
    
    return std::string(std::to_string(usage) + " " + units[unitIndex]);
}

#ifdef _WIN32

#include <Windows.h>
#include <Psapi.h>

static ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
static int numProcessors;
static HANDLE self;

PROCESS_MEMORY_COUNTERS_EX GetMemoryInfo() {
    if (!initiated) InitSystem();
    
    PROCESS_MEMORY_COUNTERS_EX info;
    GetProcessMemoryInfo(self, (PROCESS_MEMORY_COUNTERS*)&info, sizeof(info));
    
    return info;
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
    
    return FormatOutput((totalVRAM - availableMemory) * 1024);
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

#elif __APPLE__

#include <mach/mach_init.h>
#include <mach/mach_host.h>
#include <mach/task.h>

static unsigned long long _previousTotalTicks = 0;
static unsigned long long _previousIdleTicks = 0;

std::string System::GetVirtualMemoryUsage() {
    struct task_basic_info t_info;
    mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;
    
    task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)&t_info, &t_info_count);
    
    return FormatOutput(int(t_info.virtual_size));
}

std::string System::GetPhysicalMemoryUsage() {
    struct task_basic_info t_info;
    mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;
    
    task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)&t_info, &t_info_count);
    
    return FormatOutput(int(t_info.resident_size));
}

std::string System::GetVRAMUsage() {
    int availableMemory;
    glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &availableMemory);
    
    return FormatOutput((totalVRAM - availableMemory) * 1024);
}

double CalculateCPULoad(unsigned long long idleTicks, unsigned long long totalTicks) {
    unsigned long long totalTicksSinceLastTime = totalTicks - _previousTotalTicks;
    unsigned long long idleTicksSinceLastTime  = idleTicks - _previousIdleTicks;
    
    _previousTotalTicks = totalTicks;
    _previousIdleTicks  = idleTicks;
    
    if (totalTicksSinceLastTime > 0) {
        return 1.0f - double(idleTicksSinceLastTime) / totalTicksSinceLastTime;
    }
    return 1.0f;
}

double System::GetCPUUsage() {
    unsigned long long totalTicks = 0;
    host_cpu_load_info_data_t cpuinfo;
    mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;
    
    host_statistics(mach_host_self(), HOST_CPU_LOAD_INFO, (host_info_t)&cpuinfo, &count);
    
    for (int i = 0; i < CPU_STATE_MAX; i++) {
        totalTicks += cpuinfo.cpu_ticks[i];
    }
    
    return CalculateCPULoad(cpuinfo.cpu_ticks[CPU_STATE_IDLE], totalTicks) * 100.0;
}

void InitSystem() {
    initiated = true;
    glGetIntegerv(GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &totalVRAM);
}

#endif