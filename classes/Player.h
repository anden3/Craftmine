#pragma once

#include <GLFW/glfw3.h>

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
std::vector<glm::vec3> Get_Chunk_Pos(glm::vec3 worldPos);

class Player {
public:
    Player();

    glm::vec3 WorldPos = glm::vec3(0.0f);
    glm::vec3 CurrentChunk = glm::vec3(0);
    glm::vec3 CurrentTile = glm::vec3(0);

    glm::vec3 Velocity;

    float SpeedModifier = 1.0f;

    bool Flying = false;
    bool Jumping = false;
    bool OnGround = false;
    bool MovedMouse = false;

    glm::dvec2 LastMousePos = glm::dvec2(0.0, 0.0);

    Camera Cam = Camera();

    void ColDetection();
    void Move(float deltaTime);

    std::vector<glm::vec3> Hitscan();

    void KeyHandler(int key, int action);
    void MouseHandler(double posX, double posY);
    void ScrollHandler(double offsetY);
    void ClickHandler(int button, int action);

private:
    void RenderChunks();
};