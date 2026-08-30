#pragma once
#include <optional>
#include "serialization/glm_vec2_meta.hpp" // IWYU pragma: keep
#include "animations/tile_animation_data.hpp"
#include "tile_map/tile_pickup_data.hpp"

struct TileData
{
    bool solid = false, deadly = false, portal = false;
    std::optional<TileAnimationData> animationData;
    std::optional<TilePickupData> pickup;
    glm::vec2 colliderOffset = glm::vec2(0.0f, 0.0f), colliderSize = glm::vec2(16.0f, 16.0f);
};
