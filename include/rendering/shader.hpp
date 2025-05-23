#pragma once

#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>

class Shader
{
public:
    Shader();
    ~Shader();
    void use() const;
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setVec2(const std::string &name, const glm::vec2 &value) const;
    void setMat4(const std::string &name, const glm::mat4 &mat) const;
    bool valid() const;
    void initByPath(const std::string &vertexPath, const std::string &fragmentPath);
    void initByCode(const std::string &vertexCode, const std::string &fragmentCode);

private:
    std::string loadFile(const std::string &path) const;
    GLuint shaderID;
    std::string compileLinkLog;
};