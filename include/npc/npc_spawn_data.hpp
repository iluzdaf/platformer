#pragma once

#include <optional>
#include <string>
#include <glm/gtc/matrix_transform.hpp>
#include "serialization/glm_vec2_meta.hpp" // IWYU pragma: keep
#include "actor/behaviors/patrol_data.hpp"

struct NpcSpawnData
{
    std::string type;
    glm::vec2 feet = glm::vec2(0.0f);

    std::optional<PatrolData> patrol;

    bool operator==(const NpcSpawnData &) const = default;
};
