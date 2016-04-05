#pragma once

#include <string>
#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Shader {
public:
    unsigned int Program = glCreateProgram();

	Shader(const char *shader);

	std::string Load_File(std::string path);
	void Add_Shader(unsigned int shader, std::string type, std::string path);

	void Link();
    void Bind();
	void Unbind();

private:
	unsigned int vShader = glCreateShader(GL_VERTEX_SHADER);
	unsigned int fShader = glCreateShader(GL_FRAGMENT_SHADER);
};