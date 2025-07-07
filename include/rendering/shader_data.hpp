#pragma once
#include <string>
#include <optional>

struct ShaderData
{
    std::optional<std::string>
        vertexPath,
        fragmentPath,
        vertexCode,
        fragmentCode;
};