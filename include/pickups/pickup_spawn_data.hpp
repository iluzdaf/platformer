#pragma once

#include <string>
#include <glm/gtc/matrix_transform.hpp>
#include "serialization/glm_vec2_meta.hpp" // IWYU pragma: keep

struct PickupSpawnData
{
    std::string type;
    glm::vec2 position = glm::vec2(0.0f);

    bool operator==(const PickupSpawnData &) const = default;
};
