#pragma once

#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const float DEFAULT_FOV = 90.0f;

class Camera {
  public:
    glm::vec3 Position = glm::vec3(0.0f);

    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;

    glm::vec3 FrontDirection;
    glm::vec3 RightDirection;

    float Yaw = 270.0f;
    float Pitch = 0.0f;
    float Zoom = DEFAULT_FOV;

    Camera();

    inline glm::mat4 GetViewMatrix() { return glm::lookAt(Position, Position + Front, Up); }
    void UpdateCameraVectors();
};
