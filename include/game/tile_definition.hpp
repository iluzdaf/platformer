#pragma once

#include "game/tile_kind.hpp"
#include "animations/tile_animation.hpp"
#include <optional>

struct TileDefinition
{
    TileKind kind;
    std::optional<TileAnimation> animation;
    std::optional<int> pickupReplaceIndex;
};
