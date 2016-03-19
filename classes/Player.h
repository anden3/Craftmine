#pragma once

#include <vector>
#include <tuple>

#include <glm/glm.hpp>

#include "Camera.h"
#include "Chunk.h"

enum Directions {
    LEFT,
    RIGHT,
    DOWN,
    UP,
    BACK,
    FRONT
};

extern std::vector<Chunk*> ChunkQueue;
std::tuple<glm::vec3, glm::vec3> Get_Chunk_Pos(glm::vec3 worldPos);

class Player {
public:
    Player();

    glm::vec3 WorldPos;
    glm::vec3 CurrentChunk;
    glm::vec3 CurrentTile;

    float SpeedModifier = 1.0f;

    Camera Cam;

    void ProcessKeyboard(Directions direction, float deltaTime);
    void ProcessMouseMovement(float xOffset, float yOffset);
    void ProcessMouseScroll(float yOffset);

private:
    void RenderChunks();
};