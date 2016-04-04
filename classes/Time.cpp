#include "Time.h"

#include <iostream>

#include <Windows.h>

const int Spacing = 50;
LARGE_INTEGER Frequency;

bool Initialized = false;

Time::Time(std::string id) {
    ID = id;

    t0 = 0;
    count = 0;
    begun_last = false;

	if (!Initialized) {
		QueryPerformanceFrequency(&Frequency);
		Initialized = false;
	}
}

void Time::Add() {
	LARGE_INTEGER t;
	QueryPerformanceCounter(&t);

    if (timings.size() == 0) {
        t0 = t.QuadPart;
        timings[0] = 0;
    }
    else {
        if (t0 != 0) {
			uint64_t ticks = t.QuadPart - t0;
			ticks *= 1000000;
			ticks /= Frequency.QuadPart;
            timings[count] = ticks;

            count += 1;
            t0 = 0;
        }
        else {
            t0 = t.QuadPart;
        }
    }
}

void Time::Stop() {
    t0 = 0;
}

void Time::Remove() {
    timings[count] = 0;
    count -= 1;
}

void Time::Get(std::string type) {
    if (type == "all") {
        if (!begun_last) {
            begun_last = true;
            std::cout << "" << std::endl;
        }

        Get("avg");
        Get("max");
        Get("min");
        Get("sum");
    }

    else if (type == "last") {
        Convert_Time(timings[count - 1], ID + " Loop " + std::to_string(count));
    }

    else if (type == "avg") {
        if (count > 0) {
			uint64_t avg = Get_Sum() / count;
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
        std::cerr << "ERROR::TIME::INVALID_GET_TYPE" << std::endl;
    }
}

uint64_t Time::Get_Sum() {
	uint64_t sum = 0;

    for (int i = 0; i <= count; i++) {
        sum += timings[i];
    }

    return sum;
}

uint64_t Time::Get_Min() {
	uint64_t min = 0;

    for (int i = 0; i <= count; i++) {
        if (timings[i] < min || min == 0) {
            if (timings[i] > 0) {
                min = timings[i];
            }
        }
    }

    return min;
}

uint64_t Time::Get_Max() {
	uint64_t max = 0;

    for (int i = 0; i < count; i++) {
        if (timings[i] > max) {
            max = timings[i];
        }
    }

    return max;
}

void Convert_Time(uint64_t t, std::string text) {
    std::string units[4] = {
            "microsecond",
            "millisecond",
            "second"
    };

    int unitIndex = 0;

    while (t >= 1000) {
        t /= 1000;
        unitIndex += 1;
    }

    std::string unit = units[unitIndex];

    std::string desc = "Time Taken (" + text + "):";
    std::string value = std::to_string(t);

    if (value != "1") {
        unit += "s";
    }

    if (value.length() > 7) {
        value = value.substr(0, 7);
    }

    for (int i = 0; i < (int) ((Spacing - desc.length()) / 4); i++) {
        desc += "\t";
    }

    std::cout << desc << value << " " << unit << "." << std::endl;
}