#pragma once

#include "serialization/glm_vec2_meta.hpp" // IWYU pragma: keep

struct TileColliderData
{
    glm::vec2 offset = glm::vec2(0.0f, 0.0f), size = glm::vec2(16.0f, 16.0f);

    bool operator==(const TileColliderData &) const = default;
};
