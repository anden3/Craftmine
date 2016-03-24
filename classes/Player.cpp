#include "Player.h"

#include <iostream>

#include "Time.h"

Time timer1("Timer 1");

const float PLAYER_BASE_SPEED  = 3.0f;
const float PLAYER_SENSITIVITY = 0.25f;
const float PLAYER_RANGE = 5.0f;

const float HITSCAN_STEP_SIZE = 0.1f;

const float MAX_FOV = 120.0f;
const float MIN_FOV = 1.0f;

const float GRAVITY = 0.004f;
const float JUMP_HEIGHT = 0.1f;

const float WIDTH = 0.2f;

const int RENDER_DISTANCE = 2;

bool CONSTRAIN_PITCH = true;

glm::vec3 lastChunk(-5);

bool keys[1024];

Player::Player() {}

void Player::ColDetection() {
    if (!ChunkMap.size()) {
        return;
    }

    glm::vec3 checkingPos = glm::vec3(WorldPos.x, ceil(WorldPos.y), WorldPos.z) + Velocity;

    std::vector<glm::vec3> chunkPos = Get_Chunk_Pos(checkingPos);
    glm::vec3 checkingChunk = chunkPos[0];
    glm::vec3 checkingTile = chunkPos[1];

    if (ChunkMap.count(checkingChunk) && ChunkMap[checkingChunk]->GetBlock(checkingTile) > 0) {
        glm::vec3 blockDist(-(checkingTile.x - CurrentTile.x), 0, -(checkingTile.z - CurrentTile.z));
        glm::vec3 result = blockDist * glm::dot(blockDist, Velocity);

        if (fabs(result.x) > fabs(Velocity.x)) {
            result.x = Velocity.x;
        }

        if (fabs(result.z) > fabs(Velocity.z)) {
            result.z = Velocity.z;
        }

        Velocity -= result;
    }

    OnGround = false;

    if (Velocity.y <= 0 && ChunkMap.count(CurrentChunk)) {
        if (ChunkMap[CurrentChunk]->GetBlock(CurrentTile) > 0) {
            OnGround = true;
            Velocity.y = 0;
        }
    }

    if (!OnGround) {
        Velocity.y -= GRAVITY;
    }
}

void Player::Move(float deltaTime) {
    Velocity.x = 0;
    Velocity.z = 0;

    if (keys[GLFW_KEY_LEFT_SHIFT]) {
        SpeedModifier = 2.0f;
    }
    else {
        SpeedModifier = 1.0f;
    }

    float speed = PLAYER_BASE_SPEED * SpeedModifier * deltaTime;

    if (Flying) {
        Velocity = glm::vec3(0);

        if (keys[GLFW_KEY_W]) {
            WorldPos += Cam.Front * speed;
        }

        if (keys[GLFW_KEY_S]) {
            WorldPos -= Cam.Front * speed;
        }

        if (keys[GLFW_KEY_A]) {
            WorldPos -= Cam.Right * speed;
        }

        if (keys[GLFW_KEY_D]) {
            WorldPos += Cam.Right * speed;
        }
    }
    else {
        if (keys[GLFW_KEY_W]) {
            Velocity += Cam.FrontDirection * speed;
        }

        if (keys[GLFW_KEY_S]) {
            Velocity -= Cam.FrontDirection * speed;
        }

        if (keys[GLFW_KEY_A]) {
            Velocity -= Cam.RightDirection * speed;
        }

        if (keys[GLFW_KEY_D]) {
            Velocity += Cam.RightDirection * speed;
        }

        if (Jumping) {
            Jumping = false;
            Velocity.y += JUMP_HEIGHT;
        }

        ColDetection();
        
        WorldPos += Velocity;
    }

    std::vector<glm::vec3> chunkPos = Get_Chunk_Pos(WorldPos);
    CurrentChunk = chunkPos[0];
    CurrentTile = chunkPos[1];

    Cam.Position = glm::vec3(WorldPos.x, WorldPos.y + 1.7, WorldPos.z);

    if (CurrentChunk != lastChunk) {
        lastChunk = CurrentChunk;
        RenderChunks();
    }
}


std::vector<glm::vec3> Player::Hitscan() {
    std::vector<glm::vec3> checkingChunkTile;
    std::vector<glm::vec3> failedScan = {glm::vec3(0)};

    glm::vec3 checkingPos;
    glm::vec3 checkingChunk;
    glm::vec3 checkingTile;

    for (float t = 0; t < PLAYER_RANGE; t += HITSCAN_STEP_SIZE) {
        checkingPos = Cam.Position + Cam.Front * t;

        checkingChunkTile = Get_Chunk_Pos(checkingPos);
        checkingChunk = checkingChunkTile[0];
        checkingTile = checkingChunkTile[1];

        if (ChunkMap.count(checkingChunk)) {
            if (ChunkMap[checkingChunk]->GetBlock(checkingTile) > 0) {
                return checkingChunkTile;
            }
        }
    }

    return failedScan;
}

void Player::KeyHandler(int key, int action) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_F) {
            Flying = !Flying;
        }

        if (key == GLFW_KEY_SPACE && OnGround) {
            Jumping = true;
        }
    }

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            keys[key] = true;
        }
        else if (action == GLFW_RELEASE) {
            keys[key] = false;
        }
    }
}

void Player::MouseHandler(double posX, double posY) {
    if (MovedMouse) {
        LastMousePos = glm::dvec2(posX, posY);
        MovedMouse = false;
    }

    Cam.Yaw += (posX - LastMousePos.x) * PLAYER_SENSITIVITY;
    Cam.Pitch += (LastMousePos.y - posY) * PLAYER_SENSITIVITY;

    if (CONSTRAIN_PITCH) {
        if (Cam.Pitch > 89.0f) {
            Cam.Pitch = 89.0f;
        }

        if (Cam.Pitch < -89.0f) {
            Cam.Pitch = -89.0f;
        }
    }


    LastMousePos = glm::dvec2(posX, posY);
    Cam.UpdateCameraVectors();
}

void Player::ScrollHandler(double offsetY) {
    if (Cam.Zoom >= MIN_FOV && Cam.Zoom <= MAX_FOV) {
        Cam.Zoom -= offsetY;
    }

    if (Cam.Zoom < MIN_FOV) {
        Cam.Zoom = MIN_FOV;
    }

    if (Cam.Zoom > MAX_FOV) {
        Cam.Zoom = MAX_FOV;
    }
}

void Player::ClickHandler(int button, int action) {
    if (action == GLFW_PRESS) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            std::vector<glm::vec3> hit = Hitscan();

            if (hit.size() == 2) {
                glm::vec3 hitChunk = hit[0];
                glm::vec3 hitTile = hit[1];

                ChunkMap[hitChunk]->RemoveBlock(hitTile);
            }
        }
    }
}

void Player::RenderChunks() {
    std::map<glm::vec3, Chunk*>::iterator it = ChunkMap.begin();

    while (it != ChunkMap.end()) {
        double dist = pow(CurrentChunk.x - it->first.x, 2) + pow(CurrentChunk.y - it->first.y, 2) + pow(CurrentChunk.z - it->first.z, 2);

        if (dist > pow(RENDER_DISTANCE, 2)) {
            delete it->second;
            it = ChunkMap.erase(it);
        }
        else {
            it++;
        }
    }

    if (ChunkMap.count(CurrentChunk) == 0) {
        ChunkQueue.push_back(new Chunk(CurrentChunk));
    }

    for (int x = (int) CurrentChunk.x - RENDER_DISTANCE; x <= CurrentChunk.x + RENDER_DISTANCE; x++) {
        for (int y = (int) CurrentChunk.y - RENDER_DISTANCE; y <= CurrentChunk.y + RENDER_DISTANCE; y++) {
            for (int z = (int) CurrentChunk.z - RENDER_DISTANCE; z <= CurrentChunk.z + RENDER_DISTANCE; z++) {
                glm::vec3 pos(x, y, z);

                if (pos != CurrentChunk && ChunkMap.count(pos) == 0) {
                    double dist = pow(CurrentChunk.x - x, 2) + pow(CurrentChunk.y - y, 2) + pow(CurrentChunk.z - z, 2);

                    if (dist <= pow(RENDER_DISTANCE, 2)) {
                        ChunkQueue.push_back(new Chunk(pos));
                    }
                }
            }
        }
    }
}