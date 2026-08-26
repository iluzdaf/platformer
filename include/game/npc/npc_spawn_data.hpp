#pragma once

#include <string>
#include <glm/gtc/matrix_transform.hpp>
#include "serialization/glm_ivec2_meta.hpp" // IWYU pragma: keep

struct NpcSpawnData
{
    std::string type;
    glm::ivec2 tilePosition = glm::ivec2(0, 0);

    bool operator==(const NpcSpawnData &) const = default;
};
