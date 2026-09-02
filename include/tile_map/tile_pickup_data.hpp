#pragma once

#include "serialization/tile_index_meta.hpp" // IWYU pragma: keep
#include "tile_map/tile_index.hpp"

#include <optional>

struct TilePickupData
{
    TileIndex replaceIndex;
    std::optional<int> scoreDelta;

    bool operator==(const TilePickupData &) const = default;
};
