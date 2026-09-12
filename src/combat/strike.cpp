#include <optional>
#include <glm/gtc/matrix_transform.hpp>
#include "combat/hit.hpp"
#include "combat/hurting.hpp"
#include "combat/strike.hpp"
#include "physics/aabb.hpp"

std::optional<Hit> hitFrom(
    const Hurting &hurting,
    glm::vec2 attackerFeet,
    const AABB &targetBox,
    glm::vec2 targetFeet)
{
    if (!hurting.box.intersects(targetBox))
        return std::nullopt;

    float away = targetFeet.x < attackerFeet.x ? -1.0f : 1.0f;
    float direction = hurting.direction != 0.0f ? hurting.direction : away;
    return Hit{hurting.damage, glm::vec2(direction, 0.0f), false};
}
