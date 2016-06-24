#pragma once

#include <map>
#include <string>
#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader {
public:
    unsigned int Program = glCreateProgram();

    Shader(const char *shader);
    void Add_Shader(unsigned int shader, std::string type, std::string path);

    void Link();

    inline void Bind() { glUseProgram(Program); }
    inline void Unbind() { glUseProgram(0); }

    int Get_Location(const std::string name);

    inline void Upload(const std::string &name, const int &input) {
        glProgramUniform1i(Program, Get_Location(name), input);
    }
    inline void Upload(const std::string &name, const float &input) {
        glProgramUniform1f(Program, Get_Location(name), input);
    }
    inline void Upload(const std::string &name, const glm::vec2 &input) {
        glProgramUniform2f(Program, Get_Location(name), input.x, input.y);
    }
    inline void Upload(const std::string &name, const glm::vec3 &input) {
        glProgramUniform3f(Program, Get_Location(name), input.x, input.y, input.z);
    }
    inline void Upload(const std::string &name, const glm::vec4 &input) {
        glProgramUniform4f(Program, Get_Location(name), input.x, input.y, input.z, input.w);
    }
    inline void Upload(const std::string &name, const glm::mat4 &input) {
        glProgramUniformMatrix4fv(Program, Get_Location(name), 1, false, glm::value_ptr(input));
    }

private:
    unsigned int vShader = glCreateShader(GL_VERTEX_SHADER);
    unsigned int fShader = glCreateShader(GL_FRAGMENT_SHADER);

    std::map<std::string, int> UniformLocations;

    std::string Load_File(std::string path);
};
