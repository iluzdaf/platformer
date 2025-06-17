#pragma once
#include <string>

struct ShaderData
{
    std::optional<std::string> vertexPath;
    std::optional<std::string> fragmentPath;
    std::optional<std::string> vertexCode;
    std::optional<std::string> fragmentCode;
};