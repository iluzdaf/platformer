#pragma once
#include <string>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
class ShaderData;

class Shader
{
public:
    explicit Shader(const ShaderData &shaderData);
    ~Shader();
    void use() const;
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setVec2(const std::string &name, const glm::vec2 &value) const;
    void setMat4(const std::string &name, const glm::mat4 &mat) const;
    void setVec4(const std::string &name, const glm::vec4 &value) const;

private:
    GLuint shaderID = 0;
    std::string compileLinkLog;

    std::string loadFile(const std::string &path) const;
    void initByShaderFile(const std::string &vertexPath, const std::string &fragmentPath);
    void initByCode(const std::string &vertexCode, const std::string &fragmentCode);
};