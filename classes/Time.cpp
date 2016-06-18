#include "Time.h"

#include <iostream>

const int SPACING = 50;
const std::string UNITS[4] = { "nanosecond", "microsecond", "millisecond", "second" };

static bool Initialized = false;

void Convert_Time(uint64_t t, std::string text) {
    int unitIndex = 0;

    while (t >= 1000) {
        t /= 1000;
        unitIndex += 1;
    }

    std::string desc = "Time Taken (" + text + "):";
    std::string value = std::to_string(t);
    std::string unit = UNITS[unitIndex] + (value != "1" ? "s" : "");

    if (value.length() > 7) {
        value = value.substr(0, 7);
    }

    for (int i = 0; i < static_cast<int>((SPACING - desc.length()) / 4); ++i) {
        desc += "\t";
    }

    printf("%s%s %s.\n", desc.c_str(), value.c_str(), unit.c_str());
}

void Time::Get(std::string type) {
    if (type == "all") {
        if (!BegunLast) {
            BegunLast = true;
            puts("");
        }

        Get("avg");
        Get("max");
        Get("min");
        Get("sum");
    }

    else if (type == "last") {
        Convert_Time(Timings[Count - 1], ID + " Loop " + std::to_string(Count));
    }

    else if (type == "avg") {
        if (Count > 0) {
            uint64_t avg = Get_Sum() / static_cast<uint64_t>(Count);
            Convert_Time(avg, ID + " Average");
        }
        else {
            Get("last");
        }
    }

    else if (type == "min") {
        Convert_Time(Get_Min(), ID + " Min");
    }

    else if (type == "max") {
        Convert_Time(Get_Max(), ID + " Max");
    }

    else if (type == "sum") {
        Convert_Time(Get_Sum(), ID + " Sum");
    }

    else {
        puts("ERROR::TIME::INVALID_GET_TYPE");
    }
}

uint64_t Time::Get_Sum() {
    uint64_t sum = 0;

    for (int i = 0; i <= Count; ++i) {
        sum += Timings[i];
    }

    return sum;
}

uint64_t Time::Get_Min() {
    uint64_t min = 0;

    for (int i = 0; i <= Count; ++i) {
        if (Timings[i] < min || min == 0) {
            if (Timings[i] > 0) {
                min = Timings[i];
            }
        }
    }

    return min;
}

uint64_t Time::Get_Max() {
    uint64_t max = 0;

    for (int i = 0; i < Count; ++i) {
        if (Timings[i] > max) {
            max = Timings[i];
        }
    }

    return max;
}

#ifdef _WIN32
#include <Windows.h>

LARGE_INTEGER Frequency;
double Multiplier;

Time::Time(std::string id) {
    ID = id;

    if (!Initialized) {
        QueryPerformanceFrequency(&Frequency);
        Multiplier = 1000000000 / Frequency.QuadPart;
        Initialized = true;
    }
}

void Time::Add() {
    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);

    if (Timings.size() == 0) {
        T0 = t.QuadPart;
        Timings[0] = 0;
    }
    else {
        if (T0 != 0) {
            Timings[Count++] = (t.QuadPart - T0) * Multiplier;
        }

        T0 = (T0 == 0) ? t.QuadPart : 0;
    }
}

#elif __APPLE__
#include <mach/mach_time.h>

static mach_timebase_info_data_t s_timebase_info;

Time::Time(std::string id) {
    ID = id;

    if (!Initialized) {
        if (s_timebase_info.denom == 0) {
            static_cast<void>(mach_timebase_info(&s_timebase_info));
        }

        Initialized = true;
    }
}

void Time::Add() {
    uint64_t t = mach_absolute_time();

    if (Timings.size() == 0) {
        T0 = t;
        Timings[0] = 0;
    }
    else {
        if (T0 != 0) {
            Timings[Count++] = ((t - T0) * s_timebase_info.numer) / s_timebase_info.denom;
        }

        T0 = (T0 == 0) ? t : 0;
    }
}

#endif
