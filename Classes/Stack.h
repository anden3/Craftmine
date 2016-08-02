#pragma once

struct Stack {
    Stack() {
        Type = 0;
        Size = 0;
    }

    Stack(std::string type, int size = 1) : Size(size) {
        size_t delimPos = type.find(':');
        Type = std::stoi(type.substr(0, delimPos));

        if (delimPos != std::string::npos) {
            Data = std::stoi(type.substr(delimPos + 1));
        }
    }

    Stack(int type, int data, int size) : Type(type), Size(size), Data(data) {}
    Stack(int type, int size = 1) : Type(type), Size(size) {}

    void Clear() {
        Type = 0;
        Size = 0;
        Data = 0;
    }

    int Type;
    int Size;
    int Data = 0;
};