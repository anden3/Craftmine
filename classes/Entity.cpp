#include "Entity.h"

#include <random>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

std::vector<EntityInstance*> Entities;

EntityInstance::EntityInstance(glm::vec3 pos, int type, glm::vec3 velocity) {
    Position = pos + glm::vec3(0.5f);
    Type = type;
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    Data data;
    
    glm::vec2 texPosition = textureCoords[type];
    static float textureStep = (1.0f / 16.0f);
    
    float texStartX = textureStep * (texPosition.x - 1.0f);
    float texStartY = textureStep * (texPosition.y - 1.0f);
    
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            Extend(data, std::vector<float> {
                (vertices[i][j][0] - 0.5f) * ENTITY_SCALE, (vertices[i][j][1] - 0.5f) * ENTITY_SCALE, (vertices[i][j][2] - 0.5f) * ENTITY_SCALE
            });
            
            if (type == 2) {
                data.push_back(textureStep * (grassTextures[i].x - 1.0f) + tex_coords[i][j][0] * textureStep);
                data.push_back(textureStep * (grassTextures[i].y - 1.0f) + tex_coords[i][j][1] * textureStep);
            }
            else {
                data.push_back(texStartX + tex_coords[i][j][0] * textureStep);
                data.push_back(texStartY + tex_coords[i][j][1] * textureStep);
            }
        }
    }
    
    Upload_Data(VBO, data);
    VertexCount = int(data.size() / 5);
    
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 5 * sizeof(float), (void*)0);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, false, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
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
    
    modelShader->Bind();
    
    glm::mat4 model;
    model = glm::translate(model, Position);
    model = glm::rotate(model, glm::radians(Rotation), glm::vec3(0, 1, 0));
    
    glUniform1i(glGetUniformLocation(modelShader->Program, "lightLevel"), lightLevel);
    glUniformMatrix4fv(glGetUniformLocation(modelShader->Program, "model"), 1, false, glm::value_ptr(model));
    
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, VertexCount);
    glBindVertexArray(0);
    
    modelShader->Unbind();
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