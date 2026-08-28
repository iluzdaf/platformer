#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <vector>

struct JumpArc
{
    float holdDuration = 0.0f;
    float holdFraction = 1.0f;
    std::vector<glm::vec2> offsets;

    bool operator==(const JumpArc &) const = default;
};
