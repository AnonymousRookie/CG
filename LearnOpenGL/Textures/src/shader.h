#ifndef SHADERS_SHADER_H
#define SHADERS_SHADER_H

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <glad\glad.h>

class Shader
{
public:
    Shader(const char* vertexPath, const char* fragmentPath);
    void use();

    void setInt(const std::string& name, int value) const;

private:
    void checkCompileErrors(unsigned int shader, const std::string& type);

private:
    unsigned int ID;
};

#endif  // SHADERS_SHADER_H