#pragma once

#include <string>

#define GLM_SWIZZLE
#include <glm/glm.hpp>

class NetworkClient {
  public:
    void Init(std::string name);
    void Update(unsigned int timeout = 0);

    void Connect(std::string host, int port);
    void Disconnect();

    void Send(std::string message, int channel = 0);

    inline std::string Format_Vector(glm::ivec2 vector) { return std::to_string(vector.x) + "," + std::to_string(vector.y); }
    inline std::string Format_Vector(glm::ivec3 vector) { return std::to_string(vector.x) + "," + std::to_string(vector.y) + "," + std::to_string(vector.z); }
    inline std::string Format_Vector(glm::ivec4 vector) {
        return std::to_string(vector.x) + "," + std::to_string(vector.y) + "," + std::to_string(vector.z) + "," + std::to_string(vector.w);
    }

  private:
    std::string ClientName = "";
};
