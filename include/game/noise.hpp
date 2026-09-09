#pragma once

#include <string>
#include <string_view>
#include <glm/gtc/matrix_transform.hpp>

inline constexpr std::string_view LandingNoise = "landing";

struct Noise
{
    std::string kind;
    glm::vec2 at = glm::vec2(0.0f);

    bool operator==(const Noise &) const = default;
};
