#pragma once
#include <optional>
#include "animations/tile_animation_data.hpp"
#include "tile_map/tile_collider_data.hpp"
#include "tile_map/tile_pickup_data.hpp"

struct TileData
{
    bool solid = false, deadly = false, portal = false;
    std::optional<TileAnimationData> animationData;
    std::optional<TilePickupData> pickup;
    std::optional<TileColliderData> collider;
};
