#pragma once

#include <string>

struct Stack {
    Stack();

    Stack(std::string type);
    Stack(std::string type, int size = 1);

    Stack(int type, int data, int size) : Type(type), Size(size), Data(data) {}
    Stack(int type, int size = 1) : Type(type), Size(size) {}
    
    bool operator== (const Stack &s) const;
    bool operator!= (const Stack &s) const;
    
    void Decrease();
    void Clear();

    int Type;
    int Size;
    int Data = 0;
};