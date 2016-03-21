#include "Player.h"

static const float PLAYER_BASE_SPEED = 3.0f;
static const float PLAYER_SENSITIVITY = 0.25f;

static const float MAX_FOV = 120.0f;
static const float MIN_FOV = 1.0f;

static const int RENDER_DISTANCE = 5;

static bool CONSTRAIN_PITCH = true;

glm::vec3 lastChunk(-5);

std::tuple<glm::vec3, glm::vec3> Get_Chunk_Pos(glm::vec3 worldPos) {
    glm::vec3 chunk((int) (worldPos.x / CHUNK_SIZE), (int) (worldPos.y / CHUNK_SIZE), (int) (worldPos.z / CHUNK_SIZE));
    glm::vec3 tile((int) worldPos.x % CHUNK_SIZE, (int) worldPos.y % CHUNK_SIZE, (int) worldPos.z % CHUNK_SIZE);

    return std::make_tuple(chunk, tile);
}

Player::Player() {
    WorldPos = glm::vec3(0.0f);
    CurrentChunk = glm::vec3(0);
    CurrentTile = glm::vec3(0);

    Cam = Camera();
}

void Player::ProcessKeyboard(Directions direction, float deltaTime) {
    float velocity = PLAYER_BASE_SPEED * SpeedModifier * deltaTime;

    if (direction == FRONT) {
        WorldPos += Cam.FrontDirection * velocity;
    }

    if (direction == BACK) {
        WorldPos -= Cam.FrontDirection * velocity;
    }

    if (direction == LEFT) {
        WorldPos -= Cam.RightDirection * velocity;
    }

    if (direction == RIGHT) {
        WorldPos += Cam.RightDirection * velocity;
    }

    std::tie(CurrentChunk, CurrentTile) = Get_Chunk_Pos(WorldPos);
    Cam.Position = glm::vec3(WorldPos.x, WorldPos.y + 1.7, WorldPos.z);

    if (CurrentChunk != lastChunk) {
        lastChunk = CurrentChunk;
        RenderChunks();
    }
}

void Player::ProcessMouseMovement(float xOffset, float yOffset) {
    xOffset *= PLAYER_SENSITIVITY;
    yOffset *= PLAYER_SENSITIVITY;

    Cam.Yaw   += xOffset;
    Cam.Pitch += yOffset;

    if (CONSTRAIN_PITCH) {
        if (Cam.Pitch > 89.0f) {
            Cam.Pitch = 89.0f;
        }

        if (Cam.Pitch < -89.0f) {
            Cam.Pitch = -89.0f;
        }
    }

    Cam.UpdateCameraVectors();
}

void Player::ProcessMouseScroll(float yOffset) {
    if (Cam.Zoom >= MIN_FOV && Cam.Zoom <= MAX_FOV) {
        Cam.Zoom -= yOffset;
    }

    if (Cam.Zoom < MIN_FOV) {
        Cam.Zoom = MIN_FOV;
    }

    if (Cam.Zoom > MAX_FOV) {
        Cam.Zoom = MAX_FOV;
    }
}

void Player::RenderChunks() {
    for (auto const chunk: ChunkMap) {
        double dist = pow(CurrentChunk.x - chunk.first.x, 2) + pow(CurrentChunk.y - chunk.first.y, 2) + pow(CurrentChunk.z - chunk.first.z, 2);

        if (dist > pow(RENDER_DISTANCE, 2)) {
            delete chunk.second;
            ChunkMap.erase(chunk.first);
        }
    }

    for (int x = (int) CurrentChunk.x - RENDER_DISTANCE; x <= CurrentChunk.x + RENDER_DISTANCE; x++) {
        for (int y = (int) CurrentChunk.y - RENDER_DISTANCE; y <= CurrentChunk.y + RENDER_DISTANCE; y++) {
            for (int z = (int) CurrentChunk.z - RENDER_DISTANCE; z <= CurrentChunk.z + RENDER_DISTANCE; z++) {
                glm::vec3 pos(x, y, z);

                if (ChunkMap.count(pos) == 0) {
                    double dist = pow(CurrentChunk.x - x, 2) + pow(CurrentChunk.y - y, 2) + pow(CurrentChunk.z - z, 2);

                    if (dist <= pow(RENDER_DISTANCE, 2)) {
                        ChunkQueue.push_back(new Chunk(pos));
                    }
                }
            }
        }
    }
}