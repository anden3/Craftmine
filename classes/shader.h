#pragma once

#include <string>
#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader {
public:
    unsigned int Program = glCreateProgram();

	Shader(const char *shader);

	std::string Load_File(std::string path);
	void Add_Shader(unsigned int shader, std::string type, std::string path);

	void Link();
    
    inline void Bind() { glUseProgram(Program); }
    inline void Unbind() { glUseProgram(0); }
    
    inline void Upload(const int &location, const int &input) { glProgramUniform1i(Program, location, input); }
    inline void Upload(const int &location, const float &input) { glProgramUniform1f(Program, location, input); }
    inline void Upload(const int &location, const glm::vec2 &input) { glProgramUniform2f(Program, location, input.x, input.y); }
    inline void Upload(const int &location, const glm::vec3 &input) { glProgramUniform3f(Program, location, input.x, input.y, input.z); }
    inline void Upload(const int &location, const glm::vec4 &input) { glProgramUniform4f(Program, location, input.x, input.y, input.z, input.w); }
    inline void Upload(const int &location, const glm::mat4 &input) { glProgramUniformMatrix4fv(Program, location, 1, false, glm::value_ptr(input)); }
    
    inline void Upload(const std::string &name, const int &input) { glProgramUniform1i(Program, glGetUniformLocation(Program, name.c_str()), input); }
    inline void Upload(const std::string &name, const float &input) { glProgramUniform1f(Program, glGetUniformLocation(Program, name.c_str()), input); }
    inline void Upload(const std::string &name, const glm::vec2 &input) { glProgramUniform2f(Program, glGetUniformLocation(Program, name.c_str()), input.x, input.y); }
    inline void Upload(const std::string &name, const glm::vec3 &input) { glProgramUniform3f(Program, glGetUniformLocation(Program, name.c_str()), input.x, input.y, input.z); }
    inline void Upload(const std::string &name, const glm::vec4 &input) { glProgramUniform4f(Program, glGetUniformLocation(Program, name.c_str()), input.x, input.y, input.z, input.w); }
    inline void Upload(const std::string &name, const glm::mat4 &input) { glProgramUniformMatrix4fv(Program, glGetUniformLocation(Program, name.c_str()), 1, false, glm::value_ptr(input)); }

private:
	unsigned int vShader = glCreateShader(GL_VERTEX_SHADER);
	unsigned int fShader = glCreateShader(GL_FRAGMENT_SHADER);
};