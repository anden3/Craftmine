#include "Entity.h"

#include <random>
#include <glm/gtc/matrix_transform.hpp>

#include "main.h"
#include "Chunk.h"
#include "Blocks.h"
#include "Shader.h"
#include "Interface.h"

std::vector<EntityInstance*> Entities;

EntityInstance::EntityInstance(glm::vec3 pos, int type, int typeData, int size, glm::vec3 velocity) {
    Position = pos + glm::vec3(0.5f);
    Type = type;
    Size = size;
    BlockData = typeData;

    Data data;
    Blocks::Mesh(data, Blocks::Get_Block(type, typeData), glm::vec3(-0.5f), ENTITY_SCALE);

    EntityBuffer.Init(modelShader);
    EntityBuffer.Create(3, 3, data);

    if (velocity == glm::vec3(-100)) {
        Velocity.y += 0.05f;

        float randomAngle = (rand() / (RAND_MAX / 360.0f));

        Velocity.x += glm::cos(randomAngle) * 2;
        Velocity.z += glm::sin(randomAngle) * 2;
    }
    else {
        Velocity = velocity;
    }
}

void EntityInstance::Update() {
    Rotation += ROTATION_RATE * static_cast<float>(DeltaTime);

    // Normalize rotation angle.
    if (Rotation > 360) {
        Rotation -= 360;
    }
    else if (Rotation < 0) {
        Rotation += 360;
    }

    TimeAlive += static_cast<float>(DeltaTime);

    if (TimeAlive >= IGNORE_TIMER) {
        Can_Move = true;
    }

    Col_Check();

    // Merge nearby entities.
    for (auto const &entity : Entities) {
        // Check if the entity is empty or is itself.
        if (entity->Size == 0 || entity == this) {
            continue;
        }

        float dist = glm::distance(Position, entity->Position);

        if (dist <= MERGING_RADIUS) {
            Size += entity->Size;
            entity->Size = 0;
        }
    }
}

void EntityInstance::Col_Check() {
    Velocity.y -= GRAVITY;

    OnGround = (Velocity.y <= 0 && Is_Block(
        Position + glm::vec3(0, Velocity.y - (ENTITY_SCALE / 2), 0)
    ));

    if (OnGround) {
        Velocity.y = 0;

        if (Velocity.x != 0) {
            Velocity.x -= FRICTION * Velocity.x;
        }

        if (Velocity.z != 0) {
            Velocity.z -= FRICTION * Velocity.z;
        }
    }

    else if (Velocity.y < 0) {
        if (!Is_Block(glm::vec3(Position.x, Position.y + Velocity.y - (ENTITY_SCALE / 2), Position.z))) {
            Position.y += Velocity.y;
        }
    }
    else if (Velocity.y > 0) {
        if (!Is_Block(glm::vec3(Position.x, Position.y + Velocity.y + (ENTITY_SCALE / 2), Position.z))) {
            Position.y += Velocity.y;
        }
    }

    glm::vec3 offsets[2] = {
        glm::vec3(Velocity.x + (ENTITY_SCALE / 2) * std::copysign(1.0f, Velocity.x), 0, 0),
        glm::vec3(0, 0, Velocity.z + (ENTITY_SCALE / 2) * std::copysign(1.0f, Velocity.z))
    };

    for (int i = 0; i < 3; i += 2) {
        if (Velocity[i] == 0.0f) {
            continue;
        }

        if (!Is_Block(Position + offsets[i / 2])) {
            Position[i] += Velocity[i] * static_cast<float>(DeltaTime);
        }
    }
}

void EntityInstance::Draw() {
    glm::vec3 chunk, tile;
    std::tie(chunk, tile) = Get_Chunk_Pos(Position);

    if (!Exists(chunk)) {
        return;
    }

    int lightLevel = ChunkMap[chunk]->Get_Light(tile);

    if (lightLevel == 0) {
        if (Position.y >= ChunkMap[chunk]->Get_Top(tile)) {
            lightLevel = SUN_LIGHT_LEVEL;
        }
    }

    glm::mat4 model;
    model = glm::translate(model, Position);
    model = glm::rotate(model, glm::radians(Rotation), glm::vec3(0, 1, 0));

    modelShader->Upload("lightLevel", lightLevel);
    modelShader->Upload("model", model);
    modelShader->Upload("tex", 0);

    EntityBuffer.Draw();
}


void Entity::Spawn(glm::vec3 pos, int type, int typeData, int size, glm::vec3 velocity) {
    Entities.push_back(new EntityInstance(pos, type, typeData, size, velocity));
}

void Entity::Update() {
    for (auto const &entity : Entities) {
        entity->Update();
    }

    std::vector<EntityInstance*>::iterator it = Entities.begin();

    while (it != Entities.end()) {
        if ((*it)->Size == 0) {
            it = Entities.erase(it);
        }
        else {
            ++it;
        }
    }
}

void Entity::Draw() {
    for (auto const &entity: Entities) {
        entity->Draw();
    }
}
