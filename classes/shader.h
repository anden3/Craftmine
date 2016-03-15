#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <OpenGL/gl3.h>


class Shader {
public:
    GLuint Program;

    Shader(const GLchar* vertexPath, const GLchar* geometryPath, const GLchar* fragmentPath, bool vertActive, bool geoActive, bool fragActive) {
        std::string vertexCode, geometryCode, fragmentCode;
        std::ifstream vShaderFile, gShaderFile, fShaderFile;

        vShaderFile.exceptions(std::ifstream::badbit);
        gShaderFile.exceptions(std::ifstream::badbit);
        fShaderFile.exceptions(std::ifstream::badbit);

        try {
            std::stringstream vShaderStream, gShaderStream, fShaderStream;

            if (vertActive) {
                vShaderFile.open(vertexPath);
                vShaderStream << vShaderFile.rdbuf();
                vShaderFile.close();
                vertexCode = vShaderStream.str();
            }

            if (geoActive) {
                gShaderFile.open(geometryPath);
                gShaderStream << gShaderFile.rdbuf();
                gShaderFile.close();
                geometryCode = gShaderStream.str();
            }

            if (fragActive) {
                fShaderFile.open(fragmentPath);
                fShaderStream << fShaderFile.rdbuf();
                fShaderFile.close();
                fragmentCode = fShaderStream.str();
            }
        }
        catch (std::ifstream::failure e) {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
        }

        GLuint vertex, geometry, fragment;
        GLint success;
        GLchar infoLog[512];

        this->Program = glCreateProgram();

        if (vertActive) {
            const GLchar* vShaderCode = vertexCode.c_str();

            vertex = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vertex, 1, &vShaderCode, NULL);
            glCompileShader(vertex);

            glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);

            if (!success) {
                glGetShaderInfoLog(vertex, 512, NULL, infoLog);
                std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
            }

            glAttachShader(this->Program, vertex);
        }

        if (geoActive) {
            const GLchar* gShaderCode = geometryCode.c_str();

            geometry = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(geometry, 1, &gShaderCode, NULL);
            glCompileShader(geometry);

            glGetShaderiv(geometry, GL_COMPILE_STATUS, &success);

            if (!success) {
                glGetShaderInfoLog(geometry, 512, NULL, infoLog);
                std::cout << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n" << infoLog << std::endl;
            }

            glAttachShader(this->Program, geometry);
        }

        if (fragActive) {
            const GLchar* fShaderCode = fragmentCode.c_str();

            fragment = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fragment, 1, &fShaderCode, NULL);
            glCompileShader(fragment);

            glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);

            if (!success) {
                glGetShaderInfoLog(fragment, 512, NULL, infoLog);
                std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
            }

            glAttachShader(this->Program, fragment);
        }

        glLinkProgram(this->Program);

        glGetProgramiv(this->Program, GL_LINK_STATUS, &success);

        if (!success) {
            glGetProgramInfoLog(this->Program, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }

        if (vertActive) glDeleteShader(vertex);
        if (geoActive) glDeleteShader(geometry);
        if (fragActive) glDeleteShader(fragment);
    }
    void Use() {
        glUseProgram(this->Program);
    }
};