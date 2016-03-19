#pragma once

#include <string>
#include <map>

void Convert_Time(int64_t t, std::string text);

class Time {
public:
    std::string ID;

    Time(std::string id);

    void Add();
    void Stop();
    void Remove();
    void Get(std::string type);

private:
    uint64_t t0;
    int count;
    bool begun_last;

    std::map<int, uint64_t> timings;

    int64_t Get_Sum();
    int64_t Get_Min();
    int64_t Get_Max();
};

void Convert_Time(int64_t t, std::string text);