#include "Entity.h"

#include <random>

#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"

std::vector<EntityInstance*> Entities;

EntityInstance::EntityInstance(glm::vec3 pos, int type, glm::vec3 velocity) {
    Position = pos + glm::vec3(0.5f);
    Type = type;
    
    Data data;
    
    glm::vec2 texPosition = textureCoords[type];
    
    float texStartX = texPosition.x - 1.0f;
    float texStartY = texPosition.y - 1.0f;
    
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            if (CustomVertices.count(type)) {
                data.push_back((CustomVertices[type][i][vertices[i][j][0]].x - 0.5f) * ENTITY_SCALE);
                data.push_back((CustomVertices[type][i][vertices[i][j][1]].y - 0.5f) * ENTITY_SCALE);
                data.push_back((CustomVertices[type][i][vertices[i][j][2]].z - 0.5f) * ENTITY_SCALE);
            }
            else {
                data.push_back((vertices[i][j][0] - 0.5f) * ENTITY_SCALE);
                data.push_back((vertices[i][j][1] - 0.5f) * ENTITY_SCALE);
                data.push_back((vertices[i][j][2] - 0.5f) * ENTITY_SCALE);
            }
            
            if (CustomTexCoords.count(type)) {
                data.push_back(CustomTexCoords[type][i][tex_coords[i][j][0]].x / IMAGE_SIZE_X);
                data.push_back(CustomTexCoords[type][i][tex_coords[i][j][1]].y / IMAGE_SIZE_Y);
            }
            else if (MultiTextures.count(type)) {
                data.push_back((MultiTextures[type][i].x - 1.0f + tex_coords[i][j][0]) / IMAGE_SIZE_X);
                data.push_back((MultiTextures[type][i].y - 1.0f + tex_coords[i][j][1]) / IMAGE_SIZE_Y);
            }
            else {
                data.push_back((texStartX + tex_coords[i][j][0]) / IMAGE_SIZE_X);
                data.push_back((texStartY + tex_coords[i][j][1]) / IMAGE_SIZE_Y);
            }
        }
    }
    
    EntityBuffer.Init(modelShader);
    EntityBuffer.Create(3, 2, data);
    
    if (velocity == glm::vec3(-100)) {
        Velocity.y += 0.05;
        
        double randomAngle = (rand() / (RAND_MAX / 360.0));
        
        Velocity.x += glm::cos(randomAngle) * 2;
        Velocity.z += glm::sin(randomAngle) * 2;
    }
    else {
        Velocity = velocity;
    }
}

void EntityInstance::Update(float deltaTime) {
    Rotation += ROTATION_RATE * deltaTime;
    
    if (Rotation > 360) {
        Rotation -= 360;
    }
    
    if (Rotation < 0) {
        Rotation += 360;
    }
    
    TimeAlive += deltaTime;
    
    if (TimeAlive >= IGNORE_TIMER) {
        Can_Move = true;
    }
    
    Col_Check(deltaTime);
    
    for (auto const &entity : Entities) {
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

void EntityInstance::Col_Check(float deltaTime) {
    Velocity.y -= GRAVITY;
    
    OnGround = (Velocity.y < 0 && Is_Block(glm::vec3(Position.x, Position.y + Velocity.y - (ENTITY_SCALE / 2), Position.z)));
    
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
        glm::vec3(Velocity.x + (ENTITY_SCALE / 2) * std::copysign(1, Velocity.x), 0, 0),
        glm::vec3(0, 0, Velocity.z + (ENTITY_SCALE / 2) * std::copysign(1, Velocity.z))
    };
    
    for (int i = 0; i < 3; i += 2) {
        if (Velocity[i]) {
            glm::vec3 checkingPos = Position + offsets[i / 2];
            
            if (!Is_Block(checkingPos)) {
                Position[i] += Velocity[i] * deltaTime;
            }
        }
    }
}

void EntityInstance::Draw() {
    std::vector<glm::vec3> ChunkPos = Get_Chunk_Pos(Position);
    
    if (!Exists(ChunkPos[0])) {
        return;
    }
    
    int lightLevel = ChunkMap[ChunkPos[0]]->Get_Light(ChunkPos[1]);
    
    if (lightLevel == 0) {
        if (Position.y >= topBlocks[glm::vec2(ChunkPos[0].x, ChunkPos[0].z)][glm::vec2(ChunkPos[1].x, ChunkPos[1].z)]) {
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


void Entity::Spawn(glm::vec3 pos, int type, glm::vec3 velocity) {
    if (type == 2) {
        type = 3;
    }
    
    Entities.push_back(new EntityInstance(pos, type, velocity));
}

void Entity::Update(double deltaTime) {
    for (auto const &entity : Entities) {
        entity->Update(float(deltaTime));
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