#pragma once

#include <optional>
#include <string>
#include <glm/gtc/matrix_transform.hpp>
#include "serialization/glm_ivec2_meta.hpp" // IWYU pragma: keep

// The two ends of a patrol, walked between for as long as the level is open.
// The route between them is the pathfinder's to work out, and it may differ
// coming back: a drop one way is a climb the other.
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

    // Where it walks between. Without it, the ends of whatever it lands on.
    std::optional<PatrolPoints> patrol;

    bool operator==(const NpcSpawnData &) const = default;
};
