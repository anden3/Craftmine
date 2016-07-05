#pragma once

#define GLM_SWIZZLE
#include <glm/glm.hpp>

class VectorComparator {
public:
    template <typename T>
    bool operator() (const glm::tvec2<T> &a, const glm::tvec2<T> &b) const {
        for (int i = 0; i < 2; ++i) { if (a[i] != b[i]) { return a[i] < b[i]; }; };
        return false;
    }

    template <typename T>
    bool operator() (const glm::tvec3<T> &a, const glm::tvec3<T> &b) const {
        for (int i = 0; i < 3; ++i) { if (a[i] != b[i]) { return a[i] < b[i]; }; };
        return false;
    }

    template <typename T>
    bool operator() (const glm::tvec4<T> &a, const glm::tvec4<T> &b) const {
        for (int i = 0; i < 4; ++i) { if (a[i] != b[i]) { return a[i] < b[i]; }; };
        return false;
    }
};

class ChunkPosComparator {
public:
    template <typename T>
    bool operator() (const glm::tvec3<T> &a, const glm::tvec3<T> &b) const {
        if (a.x != b.x) return a.x < b.x;
        else if (a.z != b.z) return a.z < b.z;
        else if (a.y != b.y) return a.y > b.y;
        else return false;
    }
};