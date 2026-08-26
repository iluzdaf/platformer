#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <vector>

struct NavigationProfile
{
    glm::vec2 colliderSize = glm::vec2(8.0f, 13.0f);
    std::vector<std::vector<glm::vec2>> jumpArcs;

    bool operator==(const NavigationProfile &) const = default;
};
