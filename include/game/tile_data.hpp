#pragma once

#include "game/tile_kind.hpp"
#include "animations/tile_animation_data.hpp"
#include <optional>

struct TileData
{
    TileKind kind;
    std::optional<TileAnimationData> animationData;
    std::optional<int> pickupReplaceIndex;
};