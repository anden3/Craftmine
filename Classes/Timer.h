#pragma once

#include <string>
#include <map>

void Convert_Time(uint64_t t, std::string text);

class Time {
public:
    std::string ID;

    Time(std::string id);

    void Add();
    inline void Stop() { T0 = 0; }
    inline void Remove() { Timings[Count--] = 0; }
    void Get(std::string type);

private:
    uint64_t T0    = 0;
    int      Count = 0;
    bool BegunLast = false;

    std::map<int, uint64_t> Timings;

    uint64_t Get_Sum();
    uint64_t Get_Min();
    uint64_t Get_Max();
};

void Convert_Time(uint64_t t, std::string text);