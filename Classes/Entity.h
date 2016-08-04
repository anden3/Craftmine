#pragma once

#include "Buffer.h"

static const float ENTITY_SCALE = 0.4f;
static const float ROTATION_RATE = 10.0f;
static const float FRICTION = 0.3f;
static const float IGNORE_TIMER = 1.0f;

static const float MERGING_RADIUS = 1.0f;

const float ATTRACT_RANGE = 4.0f;
const float PICKUP_RANGE  = 1.5f;
const float ATTRACT_SPEED = 1.0f;

extern void Upload_Data(const unsigned int vbo, const Data &data);

class EntityInstance {
public:
    glm::vec3 Position;
    glm::vec3 Velocity;

    int Type;
    int Size = 1;
    int BlockData = 0;

    bool Can_Move = false;

    EntityInstance(glm::vec3 pos, int type, int typeData, glm::vec3 velocity);
    void Update();
    void Draw();

private:
    float TimeAlive = 0;
    float Rotation = 0.0f;
    bool OnGround = false;

    Buffer EntityBuffer;

    void Col_Check();
};

namespace Entity {
    void Spawn(glm::vec3 pos, int type, int typeData = 0, glm::vec3 velocity = glm::vec3(-100));
    void Update();
    void Check_Pickup(glm::vec3 playerPos);
    void Draw();
}

extern std::vector<EntityInstance*> Entities;
