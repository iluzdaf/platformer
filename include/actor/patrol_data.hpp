#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include "serialization/glm_vec2_meta.hpp" // IWYU pragma: keep

struct PatrolData
{
    glm::vec2 from = glm::vec2(0.0f);
    glm::vec2 to = glm::vec2(0.0f);

    bool operator==(const PatrolData &) const = default;
};
