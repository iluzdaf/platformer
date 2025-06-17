#include <fstream>
#include <sstream>
#include <glm/gtc/type_ptr.hpp>
#include "rendering/shader.hpp"
#include "rendering/shader_data.hpp"

Shader::Shader(const ShaderData &shaderData)
{
    if (shaderData.vertexCode && shaderData.fragmentCode)
    {
        initByCode(*shaderData.vertexCode, *shaderData.fragmentCode);
    }
    else if (shaderData.vertexPath && shaderData.fragmentPath)
    {
        initByShaderFile(*shaderData.vertexPath, *shaderData.fragmentPath);
    }
    else
    {
        throw std::runtime_error("ShaderData must contain either both vertex/fragment code or both vertex/fragment paths");
    }
}

Shader::~Shader()
{
    if (shaderID != 0)
    {
        glDeleteProgram(shaderID);
        shaderID = 0;
    }
}

void Shader::use() const
{
    glUseProgram(shaderID);
}

void Shader::setBool(const std::string &name, bool value) const
{
    assert(!name.empty());

    glUniform1i(glGetUniformLocation(shaderID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string &name, int value) const
{
    assert(!name.empty());

    glUniform1i(glGetUniformLocation(shaderID, name.c_str()), value);
}

void Shader::setFloat(const std::string &name, float value) const
{
    assert(!name.empty());

    glUniform1f(glGetUniformLocation(shaderID, name.c_str()), value);
}

void Shader::setVec2(const std::string &name, const glm::vec2 &value) const
{
    assert(!name.empty());

    glUniform2fv(glGetUniformLocation(shaderID, name.c_str()), 1, &value[0]);
}

void Shader::setMat4(const std::string &name, const glm::mat4 &mat) const
{
    assert(!name.empty());

    glUniformMatrix4fv(glGetUniformLocation(shaderID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}

std::string Shader::loadFile(const std::string &path) const
{
    assert(!path.empty());

    std::ifstream file(path);
    std::stringstream buffer;
    if (file)
    {
        buffer << file.rdbuf();
    }
    return buffer.str();
}

void Shader::initByShaderFile(const std::string &vertexPath, const std::string &fragmentPath)
{
    if (shaderID != 0)
    {
        throw std::runtime_error("Shader is already initialized");
    }

    if (vertexPath.empty())
    {
        throw std::runtime_error("Vertex shader path is empty");
    }

    if (fragmentPath.empty())
    {
        throw std::runtime_error("Fragment shader path is empty");
    }

    initByCode(loadFile(vertexPath), loadFile(fragmentPath));
}

void Shader::initByCode(const std::string &vertexCode, const std::string &fragmentCode)
{
    if (shaderID != 0)
    {
        throw std::runtime_error("Shader is already initialized");
    }

    if (vertexCode.empty())
    {
        throw std::runtime_error("Vertex shader code is empty");
    }

    if (fragmentCode.empty())
    {
        throw std::runtime_error("Fragment shader code is empty");
    }

    const char *vShaderCode = vertexCode.c_str();
    const char *fShaderCode = fragmentCode.c_str();

    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, nullptr);
    glCompileShader(vertex);
    GLint success = 0;
    char log[1024];
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex, 1024, nullptr, log);
        glDeleteShader(vertex);
        throw std::runtime_error(std::string("Vertex shader compilation failed:\n") + log);
    }

    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, nullptr);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment, 1024, nullptr, log);
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        throw std::runtime_error(std::string("Fragment shader compilation failed:\n") + log);
    }

    shaderID = glCreateProgram();
    glAttachShader(shaderID, vertex);
    glAttachShader(shaderID, fragment);
    glLinkProgram(shaderID);
    glGetProgramiv(shaderID, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderID, 1024, nullptr, log);
        glDeleteProgram(shaderID);
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        shaderID = 0;
        throw std::runtime_error(std::string("Shader program linking failed:\n") + log);
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void Shader::setVec4(const std::string &name, const glm::vec4 &value) const
{
    assert(!name.empty());

    glUniform4fv(glGetUniformLocation(shaderID, name.c_str()), 1, &value[0]);
}