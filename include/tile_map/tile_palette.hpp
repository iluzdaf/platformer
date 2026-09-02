#pragma once

#include <map>
#include <optional>
#include <string>
#include "serialization/tile_index_meta.hpp" // IWYU pragma: keep
#include "tile_map/tile_data.hpp"
#include "tile_map/tile_index.hpp"
#include "tile_map/tile_set.hpp"

struct TilePalette
{
    TileSet tileSet;
    std::optional<TileIndex> scoreTile;
    std::map<int, TileData> tiles;
};

using TilePalettes = std::map<std::string, TilePalette>;
