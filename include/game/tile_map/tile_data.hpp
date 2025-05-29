#pragma once
#include <optional>
#include "game/tile_map/tile_kind.hpp"
#include "animations/tile_animation_data.hpp"

struct TileData
{
    TileKind kind;
    std::optional<TileAnimationData> animationData;
    std::optional<int> pickupReplaceIndex;
};