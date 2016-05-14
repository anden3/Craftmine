#include "Shader.h"

#include <fstream>
#include <sstream>

unsigned int shaderTypes[2] = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER };

Shader::Shader(const char *shader) {
	std::string vPath = "shaders/" + std::string(shader) + ".vert";
	std::string fPath = "shaders/" + std::string(shader) + ".frag";

	Add_Shader(vShader, "VERTEX", vPath);
	Add_Shader(fShader, "FRAGMENT", fPath);
	Link();
}

std::string Shader::Load_File(std::string path) {
	std::ifstream file(path);

	if (file) {
		std::ostringstream contents;
		contents << file.rdbuf();
		file.close();

		return contents.str().c_str();
	}

	return "ERROR";
}

void Shader::Add_Shader(unsigned int shader, std::string type, std::string path) {
	std::string shaderString = Load_File(path);
	const char* shaderCode = shaderString.c_str();
    
    if (strcmp(shaderCode, "ERROR") == 0) {
        std::cout << "ERROR::SHADER::" << type << "::FILE_NOT_SUCCESSFULLY_READ\n" << path << std::endl;
    }

	glShaderSource(shader, 1, &shaderCode, NULL);
	glCompileShader(shader);

	int success;
	char infoLog[512];

	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (!success) {
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::" << type << "::COMPILATION_FAILED  " << path << "\n" << infoLog << std::endl;
	}

	glAttachShader(Program, shader);
}

void Shader::Link() {
	int success;
	char infoLog[512];

	glLinkProgram(Program);

	glGetProgramiv(Program, GL_LINK_STATUS, &success);

	if (!success) {
		glGetProgramInfoLog(Program, 512, NULL, infoLog);
		std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}

	glDeleteShader(vShader);
	glDeleteShader(fShader);
}