#include "Camera.h"

Camera::Camera() {
    UpdateCameraVectors();
}

void Camera::UpdateCameraVectors() {
    glm::vec3 front;
    
    front.x = glm::cos(glm::radians(Yaw)) * glm::cos(glm::radians(Pitch));
    front.y = glm::sin(glm::radians(Pitch));
    front.z = glm::sin(glm::radians(Yaw)) * glm::cos(glm::radians(Pitch));

    FrontDirection = glm::normalize(glm::vec3(front.x, 0, front.z));
    RightDirection = glm::vec3(-FrontDirection.z, 0, FrontDirection.x);

    Front = glm::normalize(front);
    Right = glm::normalize(glm::cross(Front, glm::vec3(0, 1, 0)));
    Up    = glm::normalize(glm::cross(Right, Front));
}