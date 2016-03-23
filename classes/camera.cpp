#include "Camera.h"

static const float DEFAULT_FOV = 90.0f;

Camera::Camera() {
    Position = glm::vec3(0.0f);

    Yaw = 270.0f;
    Pitch = 0.0f;
    Zoom = DEFAULT_FOV;

    UpdateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() {
    return glm::lookAt(Position, Position + Front, Up);
}

void Camera::UpdateCameraVectors() {
    glm::vec3 front;

    front.x = (float) (cos(glm::radians(Yaw)) * cos(glm::radians(Pitch)));
    front.y = (float) sin(glm::radians(Pitch));
    front.z = (float) (sin(glm::radians(Yaw)) * cos(glm::radians(Pitch)));

    FrontDirection = glm::normalize(glm::vec3(front.x, 0, front.z));
    RightDirection = glm::vec3(-FrontDirection.z, 0, FrontDirection.x);

    Front = glm::normalize(front);
    Right = glm::normalize(glm::cross(Front, glm::vec3(0, 1, 0)));
    Up    = glm::normalize(glm::cross(Right, Front));
}