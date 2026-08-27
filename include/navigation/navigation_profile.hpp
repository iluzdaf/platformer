#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include "navigation/motion_arcs.hpp"

struct NavigationProfile
{
    glm::vec2 colliderSize = glm::vec2(8.0f, 13.0f);
    std::vector<JumpArc> jumpArcs;
    std::vector<std::vector<glm::vec2>> fallArcs;

    bool operator==(const NavigationProfile &) const = default;
};
