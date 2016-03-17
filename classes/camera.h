#pragma once

#include "../Variables.h"
#include "../Functions.h"

class Camera {
public:
    glm::vec3 Position = glm::vec3(0.0f);
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;

    float Yaw   = 270.0f;
    float Pitch = 0.0f;
    float Zoom = DEFAULT_FOV;

    Camera() {
        updateCameraVectors();
    }

    glm::mat4 GetViewMatrix() {
        return glm::lookAt(Position, Position + Front, Up);
    }

    void updateCameraVectors() {
        glm::vec3 front;

        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));

        Front = glm::normalize(front);
        Right = glm::normalize(glm::cross(Front, glm::vec3(0.0f, 1.0f, 0.0f)));
        Up    = glm::normalize(glm::cross(Right, Front));
    }
};