#pragma once

class Shader {
public:
    unsigned int Program;

    Shader(const char *vertexPath, const char *geometryPath, const char *fragmentPath, bool vertActive = true, bool geoActive = false, bool fragActive = true);
    void Use();
};