#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

const double DEFAULT_FOV = 90.0;

class Camera {
public:
    glm::vec3 Position = glm::vec3(0.0f);

    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;

    glm::vec3 FrontDirection;
    glm::vec3 RightDirection;

    double Yaw = 270.0;
    double Pitch = 0.0;
    double Zoom = DEFAULT_FOV;

    Camera();

    glm::mat4 GetViewMatrix();
    void UpdateCameraVectors();
};
