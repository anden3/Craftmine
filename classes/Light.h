#pragma once

#include <OpenGL/gl3.h>

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "shader.h"
#include <iostream>

struct DirLight {
    glm::vec3 Direction;

    glm::vec3 Ambient;
    glm::vec3 Diffuse;
    glm::vec3 Specular;
};

struct PointLight {
    glm::vec3 Position;

    glm::vec3 Ambient;
    glm::vec3 Diffuse;
    glm::vec3 Specular;

    float Constant;
    float Linear;
    float Quadratic;
};

struct SpotLight {
    glm::vec3 Direction;
    glm::vec3 Position;

    glm::vec3 Ambient;
    glm::vec3 Diffuse;
    glm::vec3 Specular;

    float Constant;
    float Linear;
    float Quadratic;

    float InnerCutOff;
    float OuterCutOff;
};

std::vector<DirLight> dirLights;
std::vector<PointLight> pointLights;
std::vector<SpotLight> spotLights;

namespace Light {
    void Add_Dir_Light(glm::vec3 direction, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular) {
        DirLight light;

        light.Direction = glm::normalize(direction);

        light.Ambient = ambient;
        light.Diffuse = diffuse;
        light.Specular = specular;

        dirLights.push_back(light);
    }

    void Add_Point_Light(glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float constant, float linear, float quadratic) {
        PointLight light;

        light.Position = position;

        light.Ambient = ambient;
        light.Diffuse = diffuse;
        light.Specular = specular;

        light.Constant = constant;
        light.Linear = linear;
        light.Quadratic = quadratic;

        pointLights.push_back(light);
    }

    void Add_Spot_Light(glm::vec3 direction, glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float constant, float linear, float quadratic, float innerCutOff, float outerCutOff) {
        SpotLight light;

        light.Direction = glm::normalize(direction);
        light.Position = position;

        light.Ambient = ambient;
        light.Diffuse = diffuse;
        light.Specular = specular;

        light.Constant = constant;
        light.Linear = linear;
        light.Quadratic = quadratic;

        light.InnerCutOff = innerCutOff;
        light.OuterCutOff = outerCutOff;

        spotLights.push_back(light);
    }

    void Upload_Lights(Shader shader) {
        shader.Use();

        glUniform1i(glGetUniformLocation(shader.Program, "numberDirLights"), dirLights.size());
        glUniform1i(glGetUniformLocation(shader.Program, "numberPointLights"), pointLights.size());
        glUniform1i(glGetUniformLocation(shader.Program, "numberSpotLights"), spotLights.size());

        if (dirLights.size() > 0) {
            for (int i = 0; i < dirLights.size(); i++) {
                DirLight light = dirLights[i];

                glUniform3f(glGetUniformLocation(shader.Program, ("dirLights[" + std::to_string(i) + "].direction").c_str()), light.Direction.x, light.Direction.y, light.Direction.z);

                glUniform3f(glGetUniformLocation(shader.Program, ("dirLights[" + std::to_string(i) + "].ambient").c_str()), light.Ambient.x, light.Ambient.y, light.Ambient.z);
                glUniform3f(glGetUniformLocation(shader.Program, ("dirLights[" + std::to_string(i) + "].diffuse").c_str()), light.Diffuse.x, light.Diffuse.y, light.Diffuse.z);
                glUniform3f(glGetUniformLocation(shader.Program, ("dirLights[" + std::to_string(i) + "].specular").c_str()), light.Specular.x, light.Specular.y, light.Specular.z);
            }
        }

        if (pointLights.size() > 0) {
            for (int i = 0; i < pointLights.size(); i++) {
                PointLight light = pointLights[i];

                glUniform3f(glGetUniformLocation(shader.Program, ("pointLights[" + std::to_string(i) + "].position").c_str()), light.Position.x, light.Position.y, light.Position.z);

                glUniform3f(glGetUniformLocation(shader.Program, ("pointLights[" + std::to_string(i) + "].ambient").c_str()), light.Ambient.x, light.Ambient.y, light.Ambient.z);
                glUniform3f(glGetUniformLocation(shader.Program, ("pointLights[" + std::to_string(i) + "].diffuse").c_str()), light.Diffuse.x, light.Diffuse.y, light.Diffuse.z);
                glUniform3f(glGetUniformLocation(shader.Program, ("pointLights[" + std::to_string(i) + "].specular").c_str()), light.Specular.x, light.Specular.y, light.Specular.z);

                glUniform1f(glGetUniformLocation(shader.Program, ("pointLights[" + std::to_string(i) + "].constant").c_str()), light.Constant);
                glUniform1f(glGetUniformLocation(shader.Program, ("pointLights[" + std::to_string(i) + "].linear").c_str()), light.Linear);
                glUniform1f(glGetUniformLocation(shader.Program, ("pointLights[" + std::to_string(i) + "].quadratic").c_str()), light.Quadratic);
            }
        }

        if (spotLights.size() > 0) {
            for (int i = 0; i < spotLights.size(); i++) {
                SpotLight light = spotLights[i];

                glUniform3f(glGetUniformLocation(shader.Program, ("spotLights[" + std::to_string(i) + "].position").c_str()), light.Position.x, light.Position.y, light.Position.z);

                glUniform3f(glGetUniformLocation(shader.Program, ("spotLights[" + std::to_string(i) + "].ambient").c_str()), light.Ambient.x, light.Ambient.y, light.Ambient.z);
                glUniform3f(glGetUniformLocation(shader.Program, ("spotLights[" + std::to_string(i) + "].diffuse").c_str()), light.Diffuse.x, light.Diffuse.y, light.Diffuse.z);
                glUniform3f(glGetUniformLocation(shader.Program, ("spotLights[" + std::to_string(i) + "].specular").c_str()), light.Specular.x, light.Specular.y, light.Specular.z);

                glUniform1f(glGetUniformLocation(shader.Program, ("spotLights[" + std::to_string(i) + "].constant").c_str()), light.Constant);
                glUniform1f(glGetUniformLocation(shader.Program, ("spotLights[" + std::to_string(i) + "].linear").c_str()), light.Linear);
                glUniform1f(glGetUniformLocation(shader.Program, ("spotLights[" + std::to_string(i) + "].quadratic").c_str()), light.Quadratic);

                glUniform1f(glGetUniformLocation(shader.Program, ("spotLights[" + std::to_string(i) + "].innerCutOff").c_str()), light.InnerCutOff);
                glUniform1f(glGetUniformLocation(shader.Program, ("spotLights[" + std::to_string(i) + "].outerCutOff").c_str()), light.OuterCutOff);
            }
        }
    }
}