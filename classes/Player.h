#pragma once

#include "../Variables.h"
#include "../Functions.h"
#include "camera.h"

class Player {
public:
    glm::vec3 WorldPos = glm::vec3(0.0f);
    glm::vec3 Chunk = glm::vec3(0);
    glm::vec3 Tile = glm::vec3(0);

    Camera Cam = Camera();

    void ProcessKeyboard(Directions direction, GLfloat deltaTime) {
        GLfloat velocity = PLAYER_BASE_SPEED * deltaTime;

        if (direction == FRONT) {
            WorldPos += Cam.Front * velocity;
        }

        if (direction == BACK) {
            WorldPos -= Cam.Front * velocity;
        }

        if (direction == LEFT) {
            WorldPos -= Cam.Right * velocity;
        }

        if (direction == RIGHT) {
            WorldPos += Cam.Right * velocity;
        }

        Cam.Position = glm::vec3(WorldPos.x, WorldPos.y + 1.7, WorldPos.z);
    }

    void ProcessMouseMovement(GLfloat xOffset, GLfloat yOffset) {
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

        Cam.updateCameraVectors();
    }

    void ProcessMouseScroll(float yOffset) {
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
};