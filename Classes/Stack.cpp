#include "Stack.h"

Stack::Stack() {
    Type = 0;
    Size = 0;
}

Stack::Stack(std::string type, int size) {
    Size = size;
    
    size_t delimPos = type.find(':');
    Type = std::stoi(type.substr(0, delimPos));

    if (delimPos != std::string::npos) {
        Data = std::stoi(type.substr(delimPos + 1));
    }
}

bool Stack::operator== (const Stack &s) const {
    return (Type == s.Type && Data == s.Data);
}

bool Stack::operator!= (const Stack &s) const {
    return (Type != s.Type || Data != s.Data);
}

void Stack::Decrease() {
    if (Size) {
        Size -= 1;
        
        if (Size == 0) {
            Clear();
        }
    }
}

void Stack::Clear() {
    Type = 0;
    Size = 0;
    Data = 0;
}