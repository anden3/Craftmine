#include "Time.h"

#include <iostream>
#include <mach/mach_time.h>

static const int SPACING = 50;

Time::Time(std::string id) {
    ID = id;

    t0 = 0;
    count = 0;
    begun_last = false;
}

void Time::Add() {
    uint64_t t = mach_absolute_time();

    if (timings.size() == 0) {
        t0 = t;
        timings[0] = 0;
    }
    else {
        if (t0 != 0) {
            timings[count] = t - t0;
            count += 1;
            t0 = 0;
        }
        else {
            t0 = t;
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
            int64_t avg = Get_Sum() / count;
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
        std::cout << "ERROR::TIME::INVALID_GET_TYPE" << std::endl;
    }
}

int64_t Time::Get_Sum() {
    int64_t sum = 0;

    for (int i = 0; i <= count; i++) {
        sum += timings[i];
    }

    return sum;
}

int64_t Time::Get_Min() {
    int64_t min = 0;

    for (int i = 0; i <= count; i++) {
        if (timings[i] < min || min == 0) {
            if (timings[i] > 0) {
                min = timings[i];
            }
        }
    }

    return min;
}

int64_t Time::Get_Max() {
    int64_t max = 0;

    for (int i = 0; i < count; i++) {
        if (timings[i] > max) {
            max = timings[i];
        }
    }

    return max;
}

void Convert_Time(int64_t t, std::string text) {
    std::string units[4] = {
            "nanosecond",
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

    for (int i = 0; i < (int) ((SPACING - desc.length()) / 4); i++) {
        desc += "\t";
    }

    std::cout << desc << value << " " << unit << "." << std::endl;
}