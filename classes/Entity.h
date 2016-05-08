#pragma once

#include <vector>

#include <glm/glm.hpp>

static const float ENTITY_SCALE = 0.5f;

class Entity {
public:
    Entity(int type);
    void Update();

private:
    unsigned int VAO, VBO;
};