#pragma once

#include <optional>
#include <glm/gtc/matrix_transform.hpp>
#include "combat/hit.hpp"
#include "combat/hurting.hpp"
#include "physics/aabb.hpp"

std::optional<Hit> hitFrom(
    const Hurting &hurting,
    glm::vec2 attackerFeet,
    const AABB &targetBox,
    glm::vec2 targetFeet);
