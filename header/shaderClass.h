#pragma once

#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cerrno>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp> 

std::string get_file_contents(const char* filename);

class Shader
{
    public:
    GLuint ID;
    Shader(const char* vertexFile, const char* fragmentFile);

    void Activate();
    void Delete();
    void setMat4(const std::string& name, const glm::mat4& matrix);
    void setVec3(const std::string &name, const glm::vec3 &value) const;
};