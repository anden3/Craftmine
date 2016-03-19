#pragma once

class Shader {
public:
    unsigned int Program;

    Shader(const char *vertexPath, const char *geometryPath, const char *fragmentPath, bool vertActive, bool geoActive, bool fragActive);
    void Use();
};