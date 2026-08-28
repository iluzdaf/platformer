#pragma once

#include <optional>
#include <string>
#include <glm/gtc/matrix_transform.hpp>
#include "serialization/glm_ivec2_meta.hpp" // IWYU pragma: keep

struct PatrolPoints
{
    glm::ivec2 from = glm::ivec2(0, 0);
    glm::ivec2 to = glm::ivec2(0, 0);

    bool operator==(const PatrolPoints &) const = default;
};

struct NpcSpawnData
{
    std::string type;
    glm::ivec2 tilePosition = glm::ivec2(0, 0);

    std::optional<PatrolPoints> patrol;

    bool operator==(const NpcSpawnData &) const = default;
};
