#pragma once

#include <glm/gtc/matrix_transform.hpp>

struct NavigationProfile
{
    glm::vec2 colliderSize = glm::vec2(8.0f, 13.0f);

    bool operator==(const NavigationProfile &) const = default;
};
