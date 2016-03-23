#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    glm::vec3 Position;

    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;

    glm::vec3 FrontDirection;
    glm::vec3 RightDirection;

    float Yaw;
    float Pitch;
    float Zoom;

    Camera();

    glm::mat4 GetViewMatrix();
    void UpdateCameraVectors();
};