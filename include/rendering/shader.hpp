#pragma once
#include <string>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

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
    void initByShaderFile(const std::string &vertexPath, const std::string &fragmentPath);
    void initByCode(const std::string &vertexCode, const std::string &fragmentCode);
    void setVec4(const std::string &name, const glm::vec4 &value) const;

private:
    std::string loadFile(const std::string &path) const;
    GLuint shaderID = 0;
    std::string compileLinkLog;
};