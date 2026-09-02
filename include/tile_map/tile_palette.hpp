#pragma once

#include <map>
#include <string>
#include "tile_map/tile_data.hpp"
#include "assets/sheet.hpp"

struct TilePalette
{
    Sheet tileSet;
    std::map<int, TileData> tiles;
};

using TilePalettes = std::map<std::string, TilePalette>;
