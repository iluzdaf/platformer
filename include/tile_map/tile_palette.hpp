#pragma once

#include <map>
#include <string>
#include "tile_map/tile_data.hpp"
#include "tile_map/tile_set.hpp"

struct TilePalette
{
    TileSet tileSet;
    std::map<int, TileData> tiles;
};

using TilePalettes = std::map<std::string, TilePalette>;
