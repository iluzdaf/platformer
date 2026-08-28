#pragma once

#include <optional>
#include <string>
#include <glm/gtc/matrix_transform.hpp>
#include "serialization/glm_ivec2_meta.hpp" // IWYU pragma: keep

struct PatrolData
{
    glm::ivec2 from = glm::ivec2(0, 0);
    glm::ivec2 to = glm::ivec2(0, 0);

    bool operator==(const PatrolData &) const = default;
};

struct NpcSpawnData
{
    std::string type;
    glm::ivec2 tilePosition = glm::ivec2(0, 0);

    std::optional<PatrolData> patrol;

    bool operator==(const NpcSpawnData &) const = default;
};
