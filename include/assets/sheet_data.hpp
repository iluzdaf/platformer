#pragma once

#include "assets/texture_path_data.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "serialization/glm_ivec2_meta.hpp" // IWYU pragma: keep

struct SheetData
{
    TexturePathData texture;
    glm::ivec2 cellSize = glm::ivec2(16, 16);

    bool operator==(const SheetData &) const = default;
};
