#pragma once

#include <string>
#include <glm/gtc/matrix_transform.hpp>
#include "serialization/glm_ivec2_meta.hpp" // IWYU pragma: keep

struct SheetData
{
    std::string texture;
    glm::ivec2 cellSize = glm::ivec2(16, 16);

    bool operator==(const SheetData &) const = default;
};
