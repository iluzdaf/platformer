#pragma once

#include <optional>
#include <span>
#include <glm/gtc/matrix_transform.hpp>
#include "game/noise.hpp"

struct Perceived
{
    std::optional<glm::vec2> threatFeet = std::nullopt;
    std::span<const Noise> noises = {};
};
