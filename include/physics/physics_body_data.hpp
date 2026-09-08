#pragma once

#include <optional>
#include <string>
#include <glm/gtc/matrix_transform.hpp>

struct PhysicsBodyData
{
    glm::vec2 colliderSize = glm::vec2(8, 16);
    glm::vec2 colliderOffset = glm::vec2(4, 0);
    float stepHeight = 3.0f;

    bool operator==(const PhysicsBodyData &) const = default;
};

inline constexpr float BodyHeadroomFraction = 0.25f;

inline std::optional<std::string> whyNotABody(const PhysicsBodyData &data)
{
    if (data.colliderSize.x <= 0.0f || data.colliderSize.y <= 0.0f)
        return "has a collider of no size, and that is not one anything can touch";

    if (data.stepHeight < 0.0f)
        return "has a step height below 0, and that is not a height";

    if (data.stepHeight >= data.colliderSize.y * (1.0f - BodyHeadroomFraction))
        return "has a step height of " + std::to_string(data.stepHeight) +
               ", which leaves nothing of the body to walk into a wall with";

    return std::nullopt;
}
