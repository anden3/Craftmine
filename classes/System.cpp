#include "System.h"

#include <GL/glew.h>

static int totalVRAM;

std::string System::GetVRAMUsage() {
    int availableMemory;
    glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &availableMemory);

    return FormatOutput((totalVRAM - availableMemory) * 1024);
}

#ifdef _WIN32

#include <Windows.h>
#include <Psapi.h>

static ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
static bool initiated = false;
static int numProcessors;
static HANDLE self;

PROCESS_MEMORY_COUNTERS_EX GetMemoryInfo() {
    if (!initiated) InitSystem();

    PROCESS_MEMORY_COUNTERS_EX info;
    GetProcessMemoryInfo(self, (PROCESS_MEMORY_COUNTERS*)&info, sizeof(info));

    return info;
}

std::string System::GetPhysicalMemoryUsage() {
    return FormatOutput(GetMemoryInfo().WorkingSetSize);
}

std::string System::GetVirtualMemoryUsage() {
    return FormatOutput(GetMemoryInfo().PrivateUsage);
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

static unsigned long long previousTotalTicks = 0;
static unsigned long long previousIdleTicks = 0;

task_basic_info GetMemoryInfo() {
    struct task_basic_info t_info;
    mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;

    task_info(mach_task_self(), TASK_BASIC_INFO, task_info_t(&t_info), &t_info_count);

    return t_info;
}

std::string System::GetPhysicalMemoryUsage() {
    return FormatOutput(GetMemoryInfo().resident_size);
}

std::string System::GetVirtualMemoryUsage() {
    return FormatOutput(GetMemoryInfo().virtual_size);
}

double System::GetCPUUsage() {
    unsigned long long totalTicks = 0;

    host_cpu_load_info_data_t cpuinfo;
    mach_msg_type_number_t count = static_cast<mach_msg_type_number_t>(sizeof(host_cpu_load_info_data_t) / sizeof(integer_t));

    host_statistics(mach_host_self(), HOST_CPU_LOAD_INFO, reinterpret_cast<host_info_t>(&cpuinfo), &count);

    for (int i = 0; i < CPU_STATE_MAX; i++) {
        totalTicks += cpuinfo.cpu_ticks[i];
    }

    unsigned long long idleTicks = cpuinfo.cpu_ticks[CPU_STATE_IDLE];
    unsigned long long totalTicksSinceLastTime = totalTicks - previousTotalTicks;
    unsigned long long idleTicksSinceLastTime  = idleTicks  - previousIdleTicks;

    previousTotalTicks = totalTicks;
    previousIdleTicks  = idleTicks;

    if (totalTicksSinceLastTime > 0) {
        return (1.0 - static_cast<double>(idleTicksSinceLastTime) / totalTicksSinceLastTime) * 100.0;
    }

    return 0.0;
}

#endif
